#include <iostream>
#include "src/buffer.h"

int main() {
    BufferReader *item = new BufferReader("rtsp://127.0.0.1:8554/live");
    item->startAppSink();
    return 0;
}
