// The ImageBlock members that need no renderer or decoder, kept apart from
// ImageBlock.cpp so host tests can build them.
#include <Logging.h>
#include <Memory.h>
#include <Serialization.h>

#include "ImageBlock.h"

ImageBlock::ImageBlock(const std::string& imagePath, const std::string& srcPath, int16_t width, int16_t height)
    : imagePath(imagePath), srcPath(srcPath), width(width), height(height) {}

bool ImageBlock::serialize(HalFile& file) {
  // An over-long zip path is dropped; the block then shows its placeholder.
  if (!serialization::writeString(file, imagePath) ||
      !serialization::writeString(file, serialization::keyOrEmpty(srcPath)) ||
      !serialization::tryWritePod(file, width) || !serialization::tryWritePod(file, height)) {
    LOG_ERR("IMG", "Serialization failed: image block");
    return false;
  }
  return true;
}

std::unique_ptr<ImageBlock> ImageBlock::deserialize(HalFile& file) {
  std::string path;
  std::string src;
  int16_t w = 0;
  int16_t h = 0;
  if (!serialization::tryReadString(file, path, serialization::MAX_STRING_BYTES) ||
      !serialization::tryReadString(file, src, serialization::MAX_STRING_BYTES) ||
      !serialization::tryReadPod(file, w) || !serialization::tryReadPod(file, h)) {
    LOG_ERR("IMG", "Deserialization failed: truncated or corrupt image block");
    return nullptr;
  }
  auto block = makeUniqueNoThrow<ImageBlock>(path, src, w, h);
  if (!block) LOG_ERR("IMG", "OOM: ImageBlock %u bytes", static_cast<uint32_t>(sizeof(ImageBlock)));
  return block;
}
