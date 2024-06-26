#include "cracon/cracon.hpp"

#include <cassert>
#include <exception>
#include <fstream>

#include "nlohmann/json.hpp"

namespace cracon {

File::File(std::string const &filename_config,
           std::string const &filename_default) {
  init(filename_config, filename_default);
}

bool File::write() {
  std::unique_lock lock(mutex_);
  if (should_write_config_) {
    if (write_to_file(filename_config_, config_)) {
      should_write_config_ = false;
    }
  }
  if (should_write_default_) {
    if (write_to_file(filename_default_, default_)) {
      should_write_default_ = false;
    }
  }

  return !should_write();
}

bool File::write_to_file(std::string const &filename,
                         nlohmann::json const &config) {
  try {
    if (filename.empty()) {
      CRACON_LOG_ERROR("The filename is not set, did you init?\n");
      assert(false);
      return false;
    }
    std::ofstream output_file(filename);
    output_file << config.dump(4) << std::endl;
    output_file.close();
    return true;
  } catch (std::exception const &ex) {
    CRACON_LOG_ERROR("Error writing the file %s: %s\n", filename.c_str(),
                     ex.what());
    (void)ex;
    return false;
  }
}

bool File::init(std::string const &filename_config,
                std::string const &filename_default) {
  {
    std::unique_lock lock(mutex_);
    filename_config_ = filename_config;
    filename_default_ = filename_default;
    config_ = nlohmann::json::object();
    std::ifstream file(filename_config);
    if (file.good()) {
      config_ = nlohmann::json::parse(file);
      if (config_.is_null()) {
        config_ = nlohmann::json::object();
      }
    }
  }
  return write();
}

SharedFile::SharedFile(std::string const &filename_config,
                       std::string const &filename_default) {
  init(filename_config, filename_default);
}

bool SharedFile::init(std::string const &filename_config,
                      std::string const &filename_default) {
  return file_->init(filename_config, filename_default);
}

bool SharedFile::should_write() { return file_->should_write(); }

bool SharedFile::write() { return file_->write(); }

}  // namespace cracon
