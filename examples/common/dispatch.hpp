#ifndef DISPATCH_HPP__
#define DISPATCH_HPP__

#include <sstream>
#include <cassert>
#include "hsa.h"
#include <string>

namespace amd {
namespace dispatch {

class Buffer {
private:
  size_t size;
  void *local_ptr, *system_ptr;

public:
  Buffer(size_t size_, void *local_ptr_, void *system_ptr_)
    : size(size_), local_ptr(local_ptr_), system_ptr(system_ptr_) { }
  Buffer(size_t size_, void *system_ptr_)
    : size(size_), local_ptr(system_ptr_), system_ptr(system_ptr_) { }
  void *LocalPtr() const { return local_ptr; }
  void *SystemPtr() { return system_ptr; }
  bool IsLocal() const { return local_ptr != system_ptr; }
  size_t Size() const { return size; }
};

class Dispatch {
private:
  hsa_agent_t agent;
  hsa_agent_t cpu_agent;
  uint32_t queue_size;
  hsa_queue_t* queue;
  hsa_signal_t signal;
  hsa_region_t system_region;
  hsa_region_t kernarg_region;
  hsa_region_t local_region;
  hsa_kernel_dispatch_packet_t* aql;
  uint64_t packet_index;
  void *kernarg;
  size_t kernarg_offset;
  hsa_code_object_t code_object;
  hsa_executable_t executable;

  bool Init();
  bool InitDispatch();
  bool RunDispatch();
  bool Wait();

protected:
  std::ostringstream output;
  bool Error(const char *msg);
  bool HsaError(const char *msg, hsa_status_t status = HSA_STATUS_SUCCESS);

public:
  Dispatch(int argc, const char** argv);

  void SetAgent(hsa_agent_t agent) { assert(!this->agent.handle); this->agent = agent; }
  bool HasAgent() const { return agent.handle != 0; }
  void SetCpuAgent(hsa_agent_t agent) { assert(!this->cpu_agent.handle); this->cpu_agent = agent; }
  void SetWorkgroupSize(uint16_t sizeX, uint16_t sizeY = 1, uint16_t sizeZ = 1);
  void SetGridSize(uint32_t sizeX, uint32_t sizeY = 1, uint32_t sizeZ = 1);
  void SetSystemRegion(hsa_region_t region);
  void SetKernargRegion(hsa_region_t region);
  void SetLocalRegion(hsa_region_t region);
  bool AllocateKernarg(uint32_t size);
  bool Run();
  virtual bool SetupExecutable();
  virtual bool SetupCodeObject();
  bool LoadCodeObjectFromFile(const std::string& filename);
  void* AllocateLocalMemory(size_t size);
  void* AllocateSystemMemory(size_t size);
  bool CopyToLocal(void* dest, void* src, size_t size);
  bool CopyFromLocal(void* dest, void* src, size_t size);
  Buffer* AllocateBuffer(size_t size);
  bool CopyTo(Buffer* buffer);
  bool CopyFrom(Buffer* buffer);
  virtual bool Setup() { return true; }
  virtual bool Verify() { return true; }
  void KernargRaw(const void* ptr, size_t size, size_t align);

  template <typename T>
  void Kernarg(const T* ptr, size_t size = sizeof(T), size_t align = sizeof(T)) {
    KernargRaw(ptr, size, align);
  }

  void Kernarg(Buffer* buffer);
};

}
}

#endif // DISPATCH_HPP__
