#include <stdlib.h>
#include <sys.h>
#include <malloc.c>

typedef struct line_t {
    char * line;
    struct line_t * next;
} line_t;

char * filename;
line_t * buf_head = 0;
line_t * buf_tail = 0;
int line_count;

int cmd_read(char * file);
int cmd_write(char * file);
int cmd_append();
int cmd_print(int start, int end, int line_numbers);
int cmd_delete(int start, int end);
int cmd_insert(int line);
int cmd_change(int start, int end);
int cmd_move(int start, int end, int dst);
int cmd_transfer(int start, int end, int dst);

char * _parse_address(char * cmd, int * start, int * end);
int _do_command(int nparts, char ** parts);
int _input_lines(line_t ** head, line_t ** tail);
int _cut_lines(int start, int end, line_t ** head, line_t ** tail);
void _insert_lines(line_t * head, line_t * tail, int dst);
void _free_lines(line_t * head);
int _seek(int line, line_t ** p, line_t ** pprev);

int main(int argc, char ** argv) {
    if (argc == 2) {
        int ret = cmd_read(argv[1]);
        if (ret < 0) {
            return ret;
        }
    }

    char buf[64];
    char * parts[8];

    print("*");
    fsync(STDOUT);
    int len = input(64, buf);
    int nparts = strntok(buf, ' ', parts, 8);

    while (strncmp(parts[0], "q", 2) != 0) {
        if (nparts == 0) continue;

        int ret = _do_command(nparts, parts);
        
        if (ret != 0) {
            printf("%d\r\n", ret);
        }

        print("*");
        fsync(STDOUT);
        len = input(64, buf);
        nparts = strntok(buf, ' ', parts, 8);
    }
}


char * _parse_address(char * cmd, int * start, int * end) {
    int i, j = 0;
    for (i = 0; '0' <= cmd[i] && cmd[i] <= '9'; i++);
    *start = i == 0 ? 1 : atoi(cmd, i);
    if (cmd[i] == ',') {
        i++;
        for (j = 0; '0' <= cmd[i+j] && cmd[i+j] <= '9'; j++);
        *end = j == 0 ? line_count : atoi(cmd+i, j);
    } else {
        *end = *start;
    }
    return cmd+i+j;
}


int _do_command(int nparts, char ** parts) {
    int start;
    int end;
    char * cmd = _parse_address(parts[0], &start, &end);
    if (end < start) {
        return -1;
    }
        
    switch (cmd[0]) {
    case 'a':
        return cmd_append();
    case 'w':
        return cmd_write(nparts > 1 ? parts[1] : filename);
    case 'p':
    case 'n':            
        return cmd_print(start, end, cmd[0] == 'n');
    case 'r':
        if (nparts < 2) {
            return -1;
        }
        return cmd_read(parts[1]);
    case 'd':
        return cmd_delete(start, end);
    case 'i':
        return cmd_insert(start);
    case 'c':
        return cmd_change(start, end);
    case 'm':
        if (nparts < 2) {
            return -1;
        }
        return cmd_move(start, end, atoi(parts[1], 10));
    case 't':
        if (nparts < 2) {
            return -1;
        }
        return cmd_transfer(start, end, atoi(parts[1], 10));
    }
    return -1;
}

int _input_lines(line_t ** head, line_t ** tail) {
    char buf[256];
    int len = input(256, buf);
    int new_lines = 0;
    *head = 0;
    *tail = 0;
    while (!(len == 2 && buf[0] == '.')) {
        line_t * new_line = malloc(sizeof(line_t));
        new_line->line = malloc(len);
        new_line->next = 0;
        bcopy(new_line->line, buf, len);
        if (*head == 0) *head = new_line;
        if (*tail != 0) (*tail)->next = new_line;
        *tail = new_line;
        new_lines++;
        len = input(256, buf);
    }
    return new_lines;
}

int _seek(int line, line_t ** p, line_t ** pprev) {
    int count = 0;
    for (int i = 1; i < line && *p != 0; i++) {
        *pprev = *p;
        *p = (*p)->next;
        count++;
    }
    return count;
}

void _free_lines(line_t * head) {
    while (head != 0) {
        line_t * p = head;
        head = head->next;
        free(p->line);
        free(p);
    }
}

int _cut_lines(int start, int end, line_t ** head, line_t ** tail) {
    line_t * a = buf_head;
    line_t * aprev;
    _seek(start, &a, &aprev);
    if (a == 0) {
        *head = 0;
        *tail = 0;
        return -1;
    }
    *head = a;

    line_t * b = a;
    line_t * bprev = aprev;
    // Seek to 1 past the `end`.
    int count = _seek(end-start+2, &b, &bprev);
    *tail = bprev;
    bprev->next = 0;
    aprev->next = b;

    return count;
}

void _insert_lines(line_t * head, line_t * tail, int dst) {
    line_t * dest = buf_head;
    line_t * dest_prev = 0;
    _seek(dst+1, &dest, &dest_prev);

    if (dest_prev == 0) {
        // Copy to the beginning of the buffer
        tail->next = buf_head;
        buf_head = head;
    } else {
        // Copy to the middle of the buffer
        dest_prev->next = head;
        tail->next = dest;
        if (dest == 0) {
            // Actually this was the end of the buffer
            buf_tail = tail;
        }
    }
}

int cmd_transfer(int start, int end, int dst) {
    if (start <= dst && dst <= end) {
        return -1;
    }
    if (dst > line_count) {
        return -1;
    }
    line_t * src = buf_head;
    line_t * src_prev;
    _seek(start, &src, &src_prev);
    if (src == 0) {
        return -1;
    }
    line_t * copy_head = 0;
    line_t * copy_tail = 0;
    int count = 0;
    for (int i = start; i <= end && src != 0; i++, src = src->next) {
        line_t * new_line = malloc(sizeof(line_t));
        int len = strlen(src->line)+1;
        new_line->line = malloc(len);
        bcopy(new_line->line, src->line, len);
        new_line->next = 0;
        if (copy_head == 0) copy_head = new_line;
        if (copy_tail != 0) copy_tail->next = new_line;
        copy_tail = new_line;
        count++;
    }

    _insert_lines(copy_head, copy_tail, dst);
    line_count += count;
    
    return count;
}

int cmd_move(int start, int end, int dst) {
    if (start <= dst && dst <= end) {
        return -1;
    }
    if (dst > line_count) {
        return -1;
    }

    line_t * head;
    line_t * tail;
    int count = _cut_lines(start, end, &head, &tail);
    if (dst > end) dst -= count;
    _insert_lines(head, tail, dst);

    return count;
}

int cmd_change(int start, int end) {
    cmd_delete(start, end);
    cmd_insert(start-1);
}

int cmd_delete(int start, int end) {
    line_t * p;
    line_t * q;
    int count = _cut_lines(start, end, &p, &q);
    _free_lines(p);
    line_count -= count;
    return count;
}

int cmd_print(int start, int end, int line_numbers) {
    int line = 1;
    char buf[8];
    for (line_t * cur = buf_head; cur != 0; cur = cur->next, line++) {
        if (line > end) break;
        if (line >= start) {
            if (line_numbers) {
                printf("%d ", line);
            }
            println(cur->line);
        }
    }
    return 0;
}

int cmd_write(char * file) {
    int ret = unlink(file);
    if (ret < 0 && ret != -1) {
        println("Could not overwrite file");
        return ret;
    }
    int fd = open(file, 'w');
    if (fd < 0) {
        println("Error writing file");
        return fd;
    }
    for (line_t * cur = buf_head; cur != 0; cur = cur->next) {
        fprintln(fd, cur->line);
    }
    close(fd);
    return line_count;
}

int cmd_insert(int line) {
    if (line > line_count) {
        return -1;
    }
    line_t * head;
    line_t * tail;
    int new_lines = _input_lines(&head, &tail);
    _insert_lines(head, tail, line);
    line_count += new_lines;
    return new_lines;
}

int cmd_append() {
    line_t * head;
    line_t * tail;
    int new_lines = _input_lines(&head, &tail);
    if (buf_head == 0) buf_head = head;
    if (buf_tail != 0) buf_tail->next = head;
    buf_tail = tail;
    line_count += new_lines;
    return new_lines;
}

int cmd_read(char * file) {
    filename = file;
    int fd = open(file, 'r');
    if (fd < 0) {
        println("Error reading file");
        return fd;
    }

    _free_lines(buf_head);
    buf_head = 0;
    buf_tail = 0;
    
    unsigned char buf[64];
    int pos = 0;
    int len = 0;
    int in = read(fd, buf, 64);
    while (in > 0) {
        pos += in;
        int i;
        int j = 0;
        for (i = j; i < in; i++) {
            if (buf[i] == '\n') break;
        }
        while (i < in || j == 0) {
            line_t * new_line = malloc(sizeof(line_t));
            int line_len = i-j;
            new_line->line = malloc(line_len+1);
            new_line->next = 0;
            bcopy(new_line->line, buf+j, line_len);
            new_line->line[line_len] = 0;
            if (line_len > 0 && new_line->line[line_len-1] == '\r') {
                new_line->line[line_len-1] = 0;
            }

            if (buf_head == 0) buf_head = new_line;
            if (buf_tail != 0) buf_tail->next = new_line;
            buf_tail = new_line;
            len++;
            
            j = i+1;
            for (i = j; i < in; i++) {
                if (buf[i] == '\n') break;
            }
        }

        if (j < in) {
            pos = pos - in + j;
            lseek(fd, pos);
        }
        
        in = read(fd, buf, 64);
    }
    close(fd);
    
    if (in < 0) {
        return in;
    }

    line_count = len;

    return len;
}

