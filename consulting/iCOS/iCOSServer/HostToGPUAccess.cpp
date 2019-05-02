#include "stdafx.h"
#include "myspaserver.h"

/*
#include "cuda.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
*/

GEOIPRecord *GPUGEOIPDatabase = nullptr;

/* Valid settings for device_sync method:
    0 cudaDeviceScheduleAuto  (Automatic Blocking)
    1 cudaDeviceScheduleSpin  (Spin Blocking)
    2 cudaDeviceScheduleYield  (Yield Blocking)
    3 (Undefined Blocking Method  DO NOT USE)
    4 cudaDeviceBlockingSync  (Blocking Sync Event) = low CPU utilization
*/
bool InitializeGPU(bool bPinGenericMemory, int device_sync_method)
{
	size_t ip_records;
	const GEOIPRecord *pIpRecords = iCOS::CMySocketProServer::GetSpaServer()->GeoSourceData.GetIps(ip_records);
	size_t GEOIPDatabaseSize = ip_records * sizeof(GEOIPRecord);
/*
	cudaError_t cudaStatus;

	cudaStatus = cudaSetDeviceFlags(device_sync_method | (bPinGenericMemory ? cudaDeviceMapHost : 0));
	if (cudaStatus != cudaSuccess)
	{
		fprintf(stderr,"CUDA Error setting device flags.");
		return false;
	}

	// Choose which GPU to run on, change this on a multi-GPU system.
	cudaStatus = cudaSetDevice(0);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaSetDevice failed! Do you have a CUDA-capable GPU installed?");
		return false;
	}

	// Allocate GPU buffers for GEOIPDatabase.
	cudaStatus = cudaMalloc((void**)&GPUGEOIPDatabase, GEOIPDatabaseSize );
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMalloc GPUGEOIPDatabase failed!");
		return false;
	}

	// Copy input data from host memory to GPU buffers.
	cudaStatus = cudaMemcpy(GPUGEOIPDatabase, pIpRecords, GEOIPDatabaseSize, cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
	  fprintf(stderr, "cudaMemcpy failed!");
	  return false;
	}

	cudaStatus = cudaDeviceSetCacheConfig(cudaFuncCachePreferL1);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "Could not set my Shared Memory Preference!");
		return false;
	}
*/
	return true;
}

bool DeinitializeGPU()
{
	iCOS::CGeoIpPeer::DestroyGpuIpBlockPool();
/*
	cudaError_t cudaStatus;

	// Release our GPU buffer
	if (GPUGEOIPDatabase != nullptr)
	{
		cudaStatus = cudaFree(GPUGEOIPDatabase);
		GPUGEOIPDatabase = nullptr;
		if (cudaStatus != cudaSuccess) {
    		fprintf(stderr, "cudaFree of GPUGEOIPDatabase failed!");
			return false;
		}
	}

	// cudaDeviceReset must be called before exiting in order for profiling and
    // tracing tools such as Parallel Nsight and Visual Profiler to show complete traces.
    cudaStatus = cudaDeviceReset();
    if (cudaStatus != cudaSuccess) {
    	fprintf(stderr, "cudaDeviceReset failed!");
		return false;
    }
*/
	return true;
}

namespace iCOS
{
	void CGeoIpPeer::DestroyGpuIpBlockPool() 
	{
		SPA::CAutoLock al(m_cs);
		for (std::vector<IPBlock*>::iterator it = m_vGpuIpBlock.begin(), end = m_vGpuIpBlock.end(); it != end; ++it)
		{
			IPBlock *pGpuIpBlock = *it;
//			cudaError_t cudaStatus = cudaFree(pGpuIpBlock);
		}
		m_vGpuIpBlock.clear();
	}

	IPBlock *CGeoIpPeer::LockGpuIpBlock()
	{
		IPBlock *pGpuIpBlock = nullptr;
		SPA::CAutoLock al(m_cs);
		if (m_vGpuIpBlock.size())
		{
			//get a GPU memory buffer from pool
			pGpuIpBlock = m_vGpuIpBlock.back();
			m_vGpuIpBlock.pop_back();
		}
		else
		{
			//create one if there is no ip block buffer available.
//			cudaError_t cudaStatus = cudaMalloc((void**)&pGpuIpBlock, IPBLOCK_BUFFER_SIZE);
		}
		return pGpuIpBlock;
	}

	void CGeoIpPeer::Recycle(IPBlock *pGpuIpBlock)
	{
		if (pGpuIpBlock == nullptr)
			return;
		SPA::CAutoLock al(m_cs);
		//recycle it into pool for reuse in the future
		m_vGpuIpBlock.push_back(pGpuIpBlock);
	}

	void CGeoIpPeer::DoGPULookups(IPBlock *pIPBlock, unsigned int count)
	{
		size_t ip_records;
		const GEOIPRecord *pIpRecords = iCOS::CMySocketProServer::GetSpaServer()->GeoSourceData.GetIps(ip_records);
		size_t GEOIPDatabaseSize = ip_records * sizeof(GEOIPRecord);

		//std::cout << "count = " << count << std::endl;
		
		IPBlock *gpuIpBlock = LockGpuIpBlock();

		//GPUMagic(pIPBlock, count);

		//error handlings are ignored for code clarity
		
		//Send the IP requests up to the GPU
		//cudaStatus = cudaMemcpy(gpuIpBlock, pIPBlock, count * sizeof(IPBlock), cudaMemcpyHostToDevice);
		
		//GEOIPLookupKernel<<<iBlocks, iThreads>>>(gpuIpBlock, GPUGEOIPDatabase, ip_records);

		//it is here to execute ip lookups by GPU
		
		//fake code here for test purposes
		for(unsigned int n = 0; n < count; ++n)
		{
			switch(pIPBlock[n].TargetIP)
			{
			case 2446621551: //111.123.212.145
				pIPBlock[n].LocationPtr = &m_vFakeLocation[1];
				break;
			case 2446621552: //112.123.212.145
				pIPBlock[n].LocationPtr = &m_vFakeLocation[2];
				break;
			case 2446621553: //113.123.212.145
				pIPBlock[n].LocationPtr = &m_vFakeLocation[3];
				break;
			case 16777343: //127.0.0.1
				pIPBlock[n].LocationPtr = &m_vFakeLocation.front();
				break;
			default:
				pIPBlock[n].LocationPtr = &m_vFakeLocation.back();
				break;
			}
		}

		//Copy result from GPU buffer to host memory.
		//cudaStatus = cudaMemcpy(pIPBlock, gpuIpBlock, count * sizeof(IPBlock), cudaMemcpyDeviceToHost);

		Recycle(gpuIpBlock);
	}
}