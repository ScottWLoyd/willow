
struct GapBuffer {
	char* buffer_start;
	char* point;
	char* gap_start;
	char* gap_end;
	char* buffer_end;
};

struct Session {
    GapBuffer buffer;
    char* file_name;
    bool dirty;
};

static void print_buffer(GapBuffer* buf)
{
	size_t len_first = buf->gap_start - buf->buffer_start;
	size_t len_second = buf->buffer_end - buf->gap_end;
	char* temp = (char*)malloc(len_first + len_second + 1);
    copy_string(temp, buf->buffer_start, len_first);
    copy_string(temp + len_first, buf->gap_end, len_second);
	temp[len_first + len_second] = 0;
	printf("%s\n", temp);
	free(temp);
}

static void assert_buffer_invariants(GapBuffer* buf)
{
    assert(buf->gap_start <= buf->gap_end);
    assert(buf->buffer_start <= buf->gap_start);
    assert(buf->gap_end <= buf->buffer_end);
    assert(buf->point >= buf->buffer_start && buf->point <= buf->buffer_end);
    assert(buf->point <= buf->gap_start || buf->point > buf->gap_end);
}

#define GAP_BUFFER_SIZE 2
#define GAP_BUFFER_GROW_AMOUNT 2
#define GAP_BUFFER_GAP_SIZE 2

static void init_buffer(GapBuffer* buf)
{
	buf->buffer_start = (char*)malloc(GAP_BUFFER_SIZE);
	buf->buffer_end = buf->buffer_start + GAP_BUFFER_SIZE;
	buf->point = buf->buffer_start;
	buf->gap_start = buf->buffer_start;
	buf->gap_end = buf->gap_start + GAP_BUFFER_GAP_SIZE;
	*buf->buffer_start = 0; 
    assert_buffer_invariants(buf);
}

static void free_buffer(GapBuffer* buf)
{
    if (buf->buffer_start)
    {
        free(buf->buffer_start);
    }
}

static void set_point(GapBuffer* buf, uint32_t index)
{
	buf->point = buf->buffer_start + index;
    assert_buffer_invariants(buf);
}

static void move_gap(GapBuffer* buf)
{
	if (buf->point < buf->gap_start)
	{
		size_t chars_to_move = buf->gap_start - buf->point;
		memcpy(buf->gap_end, buf->point, chars_to_move);
		buf->gap_start = buf->point;
		buf->gap_end -= chars_to_move;
	}
	else if (buf->point > buf->gap_end)
	{
		size_t chars_to_move = buf->point - buf->gap_end;
		memcpy(buf->gap_start, buf->gap_end, chars_to_move);
		buf->gap_start = buf->point;
		buf->gap_end += chars_to_move;
	}
    assert_buffer_invariants(buf);
}

static void grow_gap(GapBuffer* buf)
{
	size_t curr_size = buf->buffer_end - buf->buffer_start;
	size_t new_size = curr_size + GAP_BUFFER_GROW_AMOUNT;
	size_t gap_offset = buf->gap_start - buf->buffer_start;
	size_t point_offset = buf->point - buf->buffer_start;
	buf->buffer_start = (char*)realloc(buf->buffer_start, new_size);
	buf->buffer_end = buf->buffer_start + new_size;
	buf->point = buf->buffer_start + point_offset;
	buf->gap_start = buf->buffer_start + gap_offset;
	buf->gap_end = buf->gap_start + GAP_BUFFER_GAP_SIZE;
    assert_buffer_invariants(buf);
}

static bool insert_char(GapBuffer* buf, char c)
{
	if (buf->point != buf->gap_start)
	{
		move_gap(buf);
	}

	if (buf->gap_start == buf->gap_end)
	{
		grow_gap(buf);
	}

	*buf->gap_start++ = c;
	buf->point++;

	print_buffer(buf);
    assert_buffer_invariants(buf);
    return true;
}

static bool remove_chars(GapBuffer* buf, int dir)
{
	if (buf->point != buf->gap_start)
	{
		move_gap(buf);
	}

	if (dir > 0)
	{
        // At end of buffer - nothing to delete
        if (buf->gap_end == buf->buffer_end)
        {
            return false;
        }
		buf->gap_end += dir;
		if (buf->gap_end > buf->buffer_end)
		{
			buf->gap_end = buf->buffer_end;
		}
	}
	else
	{
        // At beginning of buffer - nothing to delete
        if (buf->gap_start == buf->buffer_start)
        {
            return false;
        }
		buf->gap_start += dir;
		if (buf->gap_start < buf->buffer_start)
		{
			buf->gap_start = buf->buffer_start;
		}
		buf->point = buf->gap_start;
	}

	print_buffer(buf);
    assert_buffer_invariants(buf);
    return true;
}

static void move_point_column(GapBuffer* buf, int amount)
{
    buf->point += amount;
    if (amount < 0 && buf->point > buf->gap_start && buf->point <= buf->gap_end)
    {
        buf->point = buf->gap_start - (buf->gap_end - buf->point);
    }
    else if (amount > 0 && buf->point > buf->gap_start && buf->point <= buf->gap_end)
    {
        buf->point = buf->gap_end + (buf->point - buf->gap_start);
    }
    assert_buffer_invariants(buf);
}

static Vec2 get_point_location(GapBuffer* buf)
{
    int new_lines = 1;
    char* ptr = buf->buffer_start;
    char* start_of_line = ptr;
    while (ptr < buf->point)
    {
        if (*ptr == '\r')
        {
            ptr++;
        }
        if (*ptr == '\n')
        {
            new_lines++;
            start_of_line = ptr + 1;
        }
        ptr++;
        if (ptr == buf->gap_start)
        {
            while (ptr++ <= buf->gap_end)
                ;
        }
    }

    int col = (int)(ptr - start_of_line) - 1;
    if (start_of_line <= buf->gap_start && ptr > buf->gap_end)
    {
        col -= (int)(buf->gap_end - buf->gap_start);
    }
    Vec2 result = { new_lines, col };
    printf("ptr pos: %d,%d\n", new_lines, col);

    return result;
}

static void test_buffer(void)
{
	GapBuffer b;
	init_buffer(&b);
	insert_char(&b, 'H');
	remove_chars(&b, -1);
	insert_char(&b, 'e');
	insert_char(&b, 'l');
	insert_char(&b, 'l');
	insert_char(&b, 'o');
	insert_char(&b, ' ');
	insert_char(&b, 'w');
	insert_char(&b, 'o');
	insert_char(&b, 'r');
	insert_char(&b, 'l');
	insert_char(&b, 'd');
	insert_char(&b, '!');
    free_buffer(&b);
}