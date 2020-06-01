#include <StreamDeckSDK/ESDMain.h>
#include "MyStreamDeckPlugin.h"

int main(int argc, const char** argv) {
  return esd_main(argc, argv, new MyStreamDeckPlugin());
}
