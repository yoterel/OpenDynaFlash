#include "OpenDynaFlash.h"
#include <iostream>

// initialize the projector
bool DynaFlashProjector::init()
{
	if (m_initialized)
	{
		if (m_verbose)
			std::cout << "DynaFlash already initialized." << std::endl;
		return true;
	}
	// apparently this is needed or else the projector will not work. perhaps dynaflash can't tolerate page faults?
	if (!SetProcessWorkingSetSizeEx(::GetCurrentProcess(), (2000UL * 1024 * 1024), (3000UL * 1024 * 1024), QUOTA_LIMITS_HARDWS_MIN_ENABLE))
	{
		if (m_verbose)
			std::cout << "SetProcessWorkingSetSize Failed!" << std::endl;
		return false;
	}
	/* create a DynaFlash instance */
	pDynaFlash = CreateDynaFlash();
	if (pDynaFlash == NULL)
	{
		return false;
	}

	/* connect to DynaFlash */
	if (pDynaFlash->Connect(m_board_index) != STATUS_SUCCESSFUL)
	{
		return false;
	}

	/* reset DynaFlash */
	if (pDynaFlash->Reset() != STATUS_SUCCESSFUL)
	{
		gracefully_close();
		return false;
	}

	DYNAFLASH_PARAM tDynaFlash_Param = {0};
	/* projection parameter setting */
	tDynaFlash_Param.dFrameRate = m_frame_rate;
	// tDynaFlash_Param.dRProportion = (double)2 * 100 / (double)5;
	// tDynaFlash_Param.dGProportion = (double)100 / (double)5;
	// tDynaFlash_Param.dBProportion = (double)2 * 100 / (double)5;
	tDynaFlash_Param.dRProportion = (double)100 / (double)3;
	tDynaFlash_Param.dGProportion = (double)100 / (double)3;
	tDynaFlash_Param.dBProportion = (double)100 / (double)3;
	tDynaFlash_Param.nBinaryMode = m_frame_mode;
	tDynaFlash_Param.nBitDepth = m_bit_depth;
	tDynaFlash_Param.nMirrorMode = (m_flip_hor ? 0 : 1);
	tDynaFlash_Param.nFlipMode = (m_flip_ver ? 1 : 0);
	if (pDynaFlash->SetParam(&tDynaFlash_Param) != STATUS_SUCCESSFUL)
	{
		gracefully_close();
		return false;
	}
	// ILLUMINANCE_MODE cur_ilum_mode;
	// if (pDynaFlash->GetIlluminance(&cur_ilum_mode) != STATUS_SUCCESSFUL)
	// {
	// 	gracefully_close();
	// 	return false;
	// }
	// std::cout << "DynaFlash current illuminance mode: " << cur_ilum_mode << std::endl;
	/* Illuminance setting */
	if (pDynaFlash->SetIlluminance(m_illum_mode) != STATUS_SUCCESSFUL)
	{
		gracefully_close();
		return false;
	}

	print_brightness_values();
	// set_brightness_values();
	// print_brightness_values();

	/* Get frame buffer for projection */
	if (pDynaFlash->AllocFrameBuffer(m_alloc_frame_buffer) != STATUS_SUCCESSFUL)
	{
		gracefully_close();
		return false;
	}

	print_version();

	if (pDynaFlash->Start() != STATUS_SUCCESSFUL)
	{
		if (m_verbose)
			std::cout << "Start Error" << std::endl;
		gracefully_close();
		return false;
	}
	m_close_signal = false;
	m_projector_thread = std::thread([this]() { //, &projector
		if (this->m_verbose)
			std::cout << "Consumer started" << std::endl;
		uint8_t *frame;
		bool sucess;
		while (!this->m_close_signal)
		{
			sucess = m_projector_queue.wait_dequeue_timed(frame, std::chrono::milliseconds(100));
			if (sucess)
			{
				this->show_buffer_internal(frame);
			}
		}
		if (this->m_verbose)
			std::cout << "Consumer finished" << std::endl;
	});
	m_initialized = true;
	return true;
}

// print out the current brightness values per channel
void DynaFlashProjector::print_brightness_values()
{
	unsigned long nDaValue[4];
	pDynaFlash->ReadDACRegister(0x00, &nDaValue[0]);
	pDynaFlash->ReadDACRegister(0x01, &nDaValue[1]);
	pDynaFlash->ReadDACRegister(0x02, &nDaValue[2]);
	pDynaFlash->ReadDACRegister(0x03, &nDaValue[3]);
	double current = ((nDaValue[0]) * (5.0 / 1024.0)) / 0.75;
	if (m_verbose)
		std::cout << "green LED current: " << current << std::endl;
	pDynaFlash->ReadDACRegister(0x04, &nDaValue[0]);
	pDynaFlash->ReadDACRegister(0x05, &nDaValue[1]);
	pDynaFlash->ReadDACRegister(0x06, &nDaValue[2]);
	pDynaFlash->ReadDACRegister(0x07, &nDaValue[3]);
	current = ((nDaValue[0]) * (5.0 / 1024.0)) / 0.75;
	if (m_verbose)
		std::cout << "red LED current: " << current << std::endl;
	pDynaFlash->ReadDACRegister(0x08, &nDaValue[0]);
	current = ((nDaValue[0]) * (5.0 / 1024.0)) / 0.067;
	if (m_verbose)
		std::cout << "blue LED current: " << current << std::endl;
}

// experimental function to set the brightness values
void DynaFlashProjector::set_brightness_values()
{
	double green_current = 0.1f;
	double Vadj = (10 * 0.075 * green_current);
	unsigned long write_value = (int)(Vadj / (5.0 / 1024.0));
	pDynaFlash->WriteDACRegister(0x00, write_value);
	pDynaFlash->WriteDACRegister(0x01, write_value);
	pDynaFlash->WriteDACRegister(0x02, write_value);
	pDynaFlash->WriteDACRegister(0x03, write_value);
	double red_current = 0.1f;
	Vadj = (10 * 0.075 * red_current);
	write_value = (int)(Vadj / (5.0 / 1024.0));
	pDynaFlash->WriteDACRegister(0x04, write_value);
	pDynaFlash->WriteDACRegister(0x05, write_value);
	pDynaFlash->WriteDACRegister(0x06, write_value);
	pDynaFlash->WriteDACRegister(0x07, write_value);
	double blue_current = 2.5f;
	Vadj = (10 * 0.0067 * blue_current);
	write_value = (int)(Vadj / (5.0 / 1024.0));
	pDynaFlash->WriteDACRegister(0x08, write_value);
}

// gracefully close the projector
void DynaFlashProjector::gracefully_close()
{
	if (m_initialized)
	{
		// shut down our internal thread, to avoid submitting more frames
		m_close_signal = true;
		m_projector_thread.join();
		m_initialized = false;

		/* stop projection */
		pDynaFlash->Stop();

		/* release framebuffer */
		pDynaFlash->ReleaseFrameBuffer();

		/* Float the mirror device */
		pDynaFlash->Float(0);

		/* disconnect the DynaFlash */
		pDynaFlash->Disconnect();

		/* release instance of DynaFlash class */
		ReleaseDynaFlash(&pDynaFlash);
		if (m_verbose)
			std::cout << "dynaflash killed." << std::endl;
	}
}

// print out the version of the driver, HW, and DLL
void DynaFlashProjector::print_version()
{
	char DriverVersion[40];
	unsigned long nVersion;

	/* get a driver version */
	pDynaFlash->GetDriverVersion(DriverVersion);

	/* get a HW version */
	pDynaFlash->GetHWVersion(&nVersion);

	/* get a DLL version */
	pDynaFlash->GetDLLVersion(&nVersion);

	if (m_verbose)
	{
		printf("DynaFlash driver Ver : %s\r\n", DriverVersion);
		printf("DynaFlash HW Ver     : %08x\r\n", nVersion);
		printf("DynaFlash DLL Ver    : %08x\r\n", nVersion);
	}
}

// will project a white frame, can block if circular buffer is full.
void DynaFlashProjector::show()
{
	show_buffer(m_white_frame.data());
}

// will project an arbitrary buffer
bool DynaFlashProjector::show_buffer(uint8_t *buffer, bool blocking)
{
	if (blocking)
	{
		m_projector_queue.wait_enqueue(buffer);
		return true;
	}
	else
	{
		return m_projector_queue.try_enqueue(buffer);
	}
}

// returns a pointer the the DynaFlash internal buffer. DO NOT RELEASE THIS MEMORY.
// not thread safe function
uint8_t *DynaFlashProjector::get_internal_buffer()
{
	uint8_t *pBuf_orig = nullptr;
	if (m_initialized)
	{
		char *pBuf_casted = static_cast<char *>(static_cast<void *>(pBuf_orig));
		if (pDynaFlash->GetFrameBuffer(&pBuf_casted, &m_nGetFrameCnt) != STATUS_SUCCESSFUL)
		{
			if (m_verbose)
				std::cout << "GetFrameBuffer Error\n";
			gracefully_close();
		}
		if (m_nGetFrameCnt == 0)
		{
			if (m_verbose)
				std::cout << "GetFrameBuffer Error: m_nGetFrameCnt == 0\n";
			gracefully_close();
		}
	}
	return pBuf_orig;
}

// will signal the projector to show the frame in the internal buffer
// assumes the buffer is filled prior to calling this function (use get_internal_buffer)
void DynaFlashProjector::post_internal_buffer()
{
	if (m_initialized)
	{
		if (pDynaFlash->PostFrameBuffer(1) != STATUS_SUCCESSFUL)
		{
			if (m_verbose)
				std::cout << "PostFrameBuffer Error\n";
			gracefully_close();
		}
	}
}

// will attempt to project an arbitrary buffer as fast as possible.
// if the internal buffer is full, frame will be dropped.
// not thread safe function
void DynaFlashProjector::show_buffer_internal(uint8_t *buffer)
{
	// NOTE: does not free the buffer memory
	if (m_initialized)
	{
		// check if frame drop is occuring
		pDynaFlash->GetStatus(&m_DynaFlashStatus);
		int dropped = m_DynaFlashStatus.InputFrames - m_DynaFlashStatus.OutputFrames;
		if (dropped > 0)
		{
			if (m_verbose)
				std::cout << "warning, frame drop is occuring (dropped: " << dropped << " so far)" << std::endl;
			return; // todo: without this, huge latency in projection. investigate why
		}
		// get pointer to the internal buffer (this location is dynamic and dictated by the driver every function call)
		if (pDynaFlash->GetFrameBuffer(&pBuf, &m_nGetFrameCnt) != STATUS_SUCCESSFUL)
		{
			if (m_verbose)
				std::cout << "GetFrameBuffer Error\n";
			gracefully_close();
		}
		if ((pBuf != NULL) && (m_nGetFrameCnt != 0))
		{
			// copy the frame to the internal buffer
			memcpy(pBuf, buffer, m_frame_size);
			// signal the projector to show the frame
			if (pDynaFlash->PostFrameBuffer(1) != STATUS_SUCCESSFUL)
			{
				if (m_verbose)
					std::cout << "PostFrameBuffer Error\n";
				gracefully_close();
			}
		}
	}
}