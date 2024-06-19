#include "Stream.h"

Stream StreamOpen(String fileName) {
    return NULL;
}

void* StreamClose(Stream stream) {
    return NULL;
}

int StreamBuffLength(Stream stream, int size) {
    return 0;
}

int StreamReadLine(String string, Stream stream, char terminator) {
    return 0;
}

int StreamReadLines(String reader, Stream stream, ConstString term) {
    return 0;
}

int StreamRead(String reader, Stream stream, int n) {
    return 0;
}

int StreamReadLineAppend(String reader, Stream stream, char terminator) {
    return 0;
}

int StreamReadLinesAppend(String reader, Stream stream, ConstString term) {
    return 0;
}

int StreamReadAppend(String string, Stream stream, int n) {
    return 0;
}

int StreamUnread(Stream stream, ConstString terminator) {
    return 0;
}

int StreamPeek(String reader, ConstStream stream) {
    return 0;
}

int StreamEOF(ConstStream stream) {
    return 0;
}

String GetChar(Stream stream) {
    return NULL;
}