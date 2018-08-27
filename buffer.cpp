
struct GapBuffer {
	char* buffer_start;
	char* point;
	char* gap_start;
	char* gap_end;
	char* buffer_end;
};

static void print_buffer(GapBuffer* buf)
{
	size_t len_first = buf->gap_start - buf->buffer_start;
	size_t len_second = buf->buffer_end - buf->gap_end;
	char* temp = (char*)malloc(len_first + len_second + 1);
	strncpy(temp, buf->buffer_start, len_first);
	strncpy(temp + len_first, buf->gap_end, len_second);
	temp[len_first + len_second] = 0;
	printf("%s\n", temp);
	free(temp);
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
}

static void set_point(GapBuffer* buf, uint32_t index)
{
	buf->point = buf->buffer_start + index;
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
}

static void insert_char(GapBuffer* buf, char c)
{
	if (buf->point != buf->gap_start)
	{
		move_gap(buf);
	}

	if (buf->gap_end - buf->gap_start == 0)
	{
		grow_gap(buf);
	}

	*buf->gap_start++ = c;
	buf->point++;

	print_buffer(buf);
}

static void remove_chars(GapBuffer* buf, int dir)
{
	if (buf->point != buf->gap_start)
	{
		move_gap(buf);
	}

	if (dir > 0)
	{
		buf->gap_end += dir;
		if (buf->gap_end > buf->buffer_end)
		{
			buf->gap_end = buf->buffer_end;
		}
	}
	else
	{
		buf->gap_start += dir;
		if (buf->gap_start < buf->buffer_start)
		{
			buf->gap_start = buf->buffer_start;
		}
		buf->point = buf->gap_start;
	}

	print_buffer(buf);
}

static char* copy_next_line(GapBuffer* buf, char* dest, size_t max_chars, char* start = NULL)
{
	if (start == NULL)
	{
		start = buf->buffer_start;
	}

	char* end = start;
	while (*end && 
		   end < buf->buffer_end && 
		   *end != '\n' && 
		   (end - start) < max_chars)
		end++;

	size_t len = 0;
	if (start < buf->gap_start && end > buf->gap_end)
	{
		char* ptr = dest;
		len = buf->gap_start - start;
		memcpy(ptr, start, len);
		ptr += len;
		memcpy(ptr, buf->gap_end, end - buf->gap_end);
		len += end - buf->gap_end;
	}
	else
	{
		len = end - start;
		memcpy(dest, start, len);
	}
	dest[len] = 0;
	return end;
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
}