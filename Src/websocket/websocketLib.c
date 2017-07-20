#include "websocket.h"
#include "LTE4G.h"

static url_t *_parse_url(const char *url, url_t *ret)
{
    char buff[256] = {0};
    int iret = sscanf(url, "%7[^://]%*c%*c%*c%127[^:]%*c%d/%1023[^?]%*c%1023s", ret->scheme, ret->hostname, &ret->port, ret->path, ret->query);
    if (2 == iret)
    {
        iret = sscanf(url, "%7[^://]%*c%*c%*c%127[^/]/%1023[^?]%*c%1023s", ret->scheme, ret->hostname, ret->path, ret->query);
        ret->port = 80;
    }
    sprintf(buff, "/%s", ret->path);
    sprintf(ret->path, "%s", buff);
    return ret;
}

static int32_t _recv_line(int32_t fd, char *buff)
{
    int32_t i = 0;
    int32_t iret = 0;
    char c = 0;
    while ('\n' != c)
    {				
        iret = recv(fd, &c, 1, 0);			
        if (iret < 0)
        {
            return iret;
        }
        buff[i++] = c;
    }

    return i - 1;
}

static int32_t _validate_headers(int32_t fd, char *key)
{
    int32_t iret = 0;
    char buff[256] = {0};
    uint32_t status = 0;
    char value[256] = {0};
    char result[256] = {0};
    char header_k[256] = {0};
    char header_v[256] = {0};
    char base64str[256] = {0};
    int32_t base64_len = 0;
    uint8_t sha1[20] = {0};
    if (_recv_line(fd, buff) < 0)
    {
					
        iret = -1;
        goto end;
    }		
    sscanf(buff, "%*s%d", &status);
		printf("status %d\r\n",status);
    if (status != 101)
    {
        iret = -1;
        goto end;
    }

    while (strcmp(buff, "\r\n") != 0)
    {
        memset(buff, 0, 256);
        memset(header_k, 0, 256);
        memset(header_v, 0, 256);
        if (_recv_line(fd, buff) < 0)
        {
            iret = -1;
            goto end;
        }
        sscanf(buff, "%256s%256s", header_k, header_v);
        if (strncmp(header_k, "Sec-WebSocket-Accept:", 256) == 0)
        {
            snprintf(result, 256, "%s", header_v);
        }
    }
    snprintf(value, 256, "%s%s", key, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    sha1Buff(value, strlen(value), sha1);
    base64_encode(sha1, 20, (uint8_t *) base64str, &base64_len);
    if (strncmp(str2lower(base64str), str2lower(result), 256) != 0)
    {
        iret = -1;
        goto end;
    }

end:
    return iret;
}

static void _getkey(char *key)
{
    int i = 0;
    char *str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    for (i = 0; i < 30; i++)
    {
        key[i] = str[rand() % 62];
    }
    key[i++] = '=';
    key[i++] = '=';
}

static int32_t _handshake(int32_t fd, const char *host, unsigned short port, const char *resource)
{
    int offset = 0;
    char header_str[512] = {0};
		HAL_Delay(5000);		
    offset += sprintf(header_str + offset, "GET %s HTTP/1.1\r\n", resource);
    offset += sprintf(header_str + offset, "Upgrade: websocket\r\n");
    offset += sprintf(header_str + offset, "Connection: Upgrade\r\n");
    offset += sprintf(header_str + offset, "Host: %s:%u\r\n", host, port);
		offset += sprintf(header_str + offset, "origin: base\r\n");
    offset += sprintf(header_str + offset, "Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n");
    offset += sprintf(header_str + offset, "Sec-WebSocket-Version: 13\r\n\r\n");
    send(fd, header_str, offset, 0);
		HAL_Delay(1000);
    return _validate_headers(fd, "x3JJHMbDL1EzLkh9GBhXDw==");
}

static void *_ANBFmask(uint32_t mask_key, void *data, int32_t len)
{
	int32_t i = 0;
	uint8_t *_m = (uint8_t *)& mask_key;
	uint8_t *_d = (uint8_t *)data;
	for (; i < len; i++)
	{
		_d[i] ^= _m[i % 4];
	}
	return _d;
}

static int32_t _create_frame(ANBF_t *frame, int fin, int rsv1, int rsv2, int rsv3, int opcode, int has_mask, void *data, int len)
{
    frame->fin = fin;
    frame->rsv1 = rsv1;
    frame->rsv2 = rsv2;
    frame->rsv3 = rsv3;
    frame->mask = has_mask;
    frame->opcode = opcode;
    frame->data = (char *)data;
    frame->length = len;

    return 0;
}

static void *_format_frame(ANBF_t *frame, int32_t *size)
{
    int offset = 0;
	  int playload = 0;
    char *frame_header = NULL;
    uint32_t masks = (uint32_t)rand() * (uint32_t)rand();
    uint16_t header =
            (frame->fin << 15) |
            (frame->rsv1 << 14) |
            (frame->rsv2 << 13) |
            (frame->rsv3 << 12) |
            (frame->opcode << 8);

    char byteLen = 0;
    if (frame->length < LENGTH_7)
    {
        header |= frame->mask << 7 | (uint8_t) frame->length;
    }
    else if (frame->length < LENGTH_16)
    {
        header |= frame->mask << 7 | 0x7e;
        byteLen = 2;
		playload = 126;
    }
    else
    {
        header |= frame->mask << 7 | 0x7f;
        byteLen = 8;
		playload = 127;
    }
		if (frame->mask)
		{
			byteLen += 4;
		}
    frame_header = (char *) malloc(sizeof (header) + byteLen + (uint32_t) frame->length);
    header = htons(header);
	  memcpy(frame_header + offset, &header, sizeof(header));
	  offset += sizeof(header);

    if (playload == 126)
    {
        uint16_t len = htons((uint16_t) frame->length);
        memcpy(frame_header + offset, &len, sizeof (len));
        offset += sizeof (len);
    }
    else if (playload == 127)
    {
        uint64_t len = htonll(frame->length);
        memcpy(frame_header + offset, &len, sizeof (len));
        offset += sizeof (len);
    }

	if (frame->mask)
	{
		memcpy(frame_header + offset, &masks, sizeof(masks));
		offset += sizeof(masks);
	}

	if (frame->mask)
	{
		int32_t i = 0;
		uint8_t *_m = (uint8_t *)& masks;
		for (; i < frame->length; i++)
		{
			frame_header[offset++] = frame->data[i] ^_m[i % 4];
			*size = offset;
		}
	}
	else
	{
		memcpy(frame_header + offset, frame->data, (uint32_t)frame->length);
		*size = offset + (uint32_t)frame->length;
	}
    return frame_header;
}

static int32_t _recv_restrict(int32_t fd, void *buff, int32_t size)
{
    int32_t offset = 0;
    int iret = 0;
    while (offset < size)
    {
        iret = recv(fd, ((char *) buff) + offset, (int32_t) (size - offset), 0);
        if (iret > 0)
        {
            offset += iret;
        }
        else
        {
            offset = -1;
            break;
        }
    }

    return offset;
}

static int32_t _recv_frame(int32_t fd, ANBF_t *frame)
{
    uint8_t b1, b2, fin, rsv1, rsv2, rsv3, opcode, has_mask;
    uint64_t frame_length = 0;
    uint16_t length_data_16 = 0;
    uint64_t length_data_64 = 0;
    uint32_t frame_mask = 0;
    uint8_t length_bits = 0;
    uint8_t frame_header[2] = {0};
    int32_t iret = 0;
    char *payload = NULL;

    iret = _recv_restrict(fd, &frame_header, 2);
    if (iret < 0)
    {
        goto end;
    }

    b1 = frame_header[0];
    b2 = frame_header[1];
    length_bits = b2 & 0x7f;
    fin = b1 >> 7 & 1;
    rsv1 = b1 >> 6 & 1;
    rsv2 = b1 >> 5 & 1;
    rsv3 = b1 >> 4 & 1;
    opcode = b1 & 0xf;
    has_mask = b2 >> 7 & 1;

    if (length_bits == 0x7e)
    {
        iret = _recv_restrict(fd, &length_data_16, 2);
        if (iret < 0)
        {
            goto end;
        }

        frame_length = ntohs(length_data_16);
    }
    else if (length_bits == 0x7f)
    {
        iret = _recv_restrict(fd, &length_data_64, 8);
        if (iret < 0)
        {
            goto end;
        }

        frame_length = ntohl(length_data_64);
    }
    else
    {
        frame_length = length_bits;
    }

    if (has_mask)
    {
        iret = _recv_restrict(fd, &frame_mask, 4);
        if (iret < 0)
        {
            goto end;
        }
    }

    if (frame_length > 0)
    {
        payload = (char *) malloc((int32_t) frame_length);
        iret = _recv_restrict(fd, payload, (int32_t) frame_length);
        if (iret < 0)
        {
            free(payload);
            goto end;
        }
    }

    if (has_mask)
    {
        _ANBFmask(frame_mask, payload, (uint32_t) frame_length);
    }

    return _create_frame(frame, fin, rsv1, rsv2, rsv3, opcode, has_mask, payload, (uint32_t) frame_length);

end:
    return -1;
}

static int32_t _send(int32_t fd, void *payload, int32_t len, int32_t opcode)
{
    int32_t length = 0;
    int32_t iret = 0;
    ANBF_t frame = {0};
    char *sendData = NULL;
    _create_frame(&frame, 1, 0, 0, 0, opcode, 0, payload, len);
    sendData = (char *) _format_frame(&frame, &length);
    iret = send(fd, sendData, length, 0);
    _free(sendData);

    return iret;
}

int32_t sendPing(wsContext_t *ctx, void *payload, int32_t len)
{
    return _send(ctx->fd, payload, len, OPCODE_PING);
}

int32_t sendPong(wsContext_t *ctx, void *payload, int32_t len)
{
    return _send(ctx->fd, payload, len, OPCODE_PONG);
}

int32_t sendCloseing(wsContext_t *ctx, uint16_t status, const char *reason)
{
    char *p = NULL;
    int len = 0;
    char payload[64] = {0};
    status = htons(status);
    p = (char *) &status;
    len = snprintf(payload, 64, "\\x%02x\\x%02x%s", p[0], p[1], reason);
    return _send(ctx->fd, payload, len, OPCODE_CLOSE);
}

int32_t recvData(wsContext_t *ctx, void *buff, int32_t len)
{
    int data_len = -1;
    int iret = -1;
    ANBF_t _frame = {0};
    ANBF_t *frame = &_frame;

    while (1)
    {
        memset(frame, 0, sizeof (frame));
        iret = _recv_frame(ctx->fd, frame);
        if (iret < 0)
        {
            goto end;
        }
        if (frame->opcode == OPCODE_TEXT || frame->opcode == OPCODE_BINARY || frame->opcode == OPCODE_CONT)
        {
            if (frame->opcode == OPCODE_CONT && NULL == ctx->cont_data)
            {
                goto end;
            }
            else if (ctx->cont_data)
            {
                ctx->cont_data = (char *) realloc(ctx->cont_data, ctx->cont_data_size + (uint32_t) frame->length);
                memcpy(ctx->cont_data + ctx->cont_data_size, frame->data, (uint32_t) frame->length);
                ctx->cont_data_size += (uint32_t) frame->length;
                _free(frame->data);
								frame->data = NULL;
            }
            else
            {
                ctx->cont_data = frame->data;
                ctx->cont_data_size = (uint32_t) frame->length;
            }

            if (frame->fin)
            {
                data_len = ctx->cont_data_size > len ? len : ctx->cont_data_size;
                memcpy(buff, ctx->cont_data, data_len);
                goto end;
            }
        }
        else if (frame->opcode == OPCODE_CLOSE)
        {
            sendCloseing(ctx, STATUS_NORMAL, "");
            close(ctx->fd);
            goto end;
        }
        else if (frame->opcode == OPCODE_PING)
        {
            sendPong(ctx, "", 0);
        }
        else
        {
            goto end;
        }
    }

end:
	if (ctx->cont_data )
	{
		_free(ctx->cont_data);
		ctx->cont_data = NULL;
	}

    ctx->cont_data_size = 0;
    return data_len;
}

int32_t sendUtf8Data(wsContext_t *ctx, void *data, int32_t len)
{
    return _send(ctx->fd, data, len, OPCODE_TEXT);
}

int32_t sendBinary(wsContext_t *ctx, void *data, int32_t len)
{
    return _send(ctx->fd, data, len, OPCODE_BINARY);
}

int32_t wsCreateConnection(wsContext_t *ctx, const char *url)
{
    url_t purl = {0};
    _parse_url(url, &purl);
    ctx->fd = ut_connect(purl.hostname, purl.port);
		printf("hand shake \r\n");
    int var = _handshake(ctx->fd, purl.hostname, purl.port, purl.path);
		printf("shake  = %d\r\n",var);
    return ctx->fd;
}

wsContext_t *wsContextNew()
{
    wsContext_t *ctx = (wsContext_t *) malloc(sizeof (wsContext_t));
    memset(ctx, 0, sizeof (wsContext_t));

    return ctx;
}

int32_t wsContextFree(wsContext_t *ctx)
{
	  sendCloseing(ctx, STATUS_NORMAL, "");
    close(ctx->fd);
    free(ctx);
    return 0;
}


uint16_t htons(uint16_t n)
{
  return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}
uint16_t ntohs(uint16_t n)
{
  return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}
uint32_t htonl(uint32_t n)
{
  return ((n & 0xff) << 24) |
    ((n & 0xff00) << 8) |
    ((n & 0xff0000UL) >> 8) |
    ((n & 0xff000000UL) >> 24);
}
uint32_t ntohl(uint32_t n)
{
  return ((n & 0xff) << 24) |
    ((n & 0xff00) << 8) |
    ((n & 0xff0000UL) >> 8) |
    ((n & 0xff000000UL) >> 24);
}
