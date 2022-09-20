#include <pspkernel.h>
#include <pspsuspend.h>
#include <pspctrl.h>
#include <pspdebug.h>
#include <pspdisplay.h>

PSP_MODULE_INFO("P5Test", 0, 1, 1);

#define P5_CHECK( addr )	(((unsigned int)addr >= (unsigned int)vol_addr) && ((unsigned int)addr < (unsigned int)vol_addr + vol_size))

int main(void)
{
	SceCtrlData	pad_data;
	unsigned int	pad_buttons;
	SceUID		mem_handle;
	unsigned int	mem_size;
	unsigned char	*mem_ptr;
	void		*vol_addr;
	unsigned int	vol_size;
	int		vol_lock_result;

	pspDebugScreenSetBackColor(0);
	pspDebugScreenSetTextColor(0xFFFFFFFF);
	pspDebugScreenInit();

	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(1);

	vol_lock_result = sceKernelVolatileMemLock(0, &vol_addr, &vol_size);
	if(vol_lock_result != 0)
	{
		pspDebugScreenPrintf("sceKernelVolatileMemLock: error ( %#08x )\n", vol_lock_result);

		pspDebugScreenPrintf("press START to exit\n");
		do
		{
			sceCtrlReadBufferPositive(&pad_data, 1);
		}
		while(!(pad_data.Buttons & PSP_CTRL_START));

		sceKernelExitGame();

		return 0;
	}
	pspDebugScreenPrintf("sceKernelVolatileMemLock: addr: %#08x size: %#08x\n", (unsigned int)vol_addr, vol_size);

	mem_size = vol_size - 256; // 256 reserved for sceKernelAllocPartitionMemory

	pspDebugScreenPrintf("Allocating size: %i\n", mem_size);

	mem_handle = sceKernelAllocPartitionMemory(5, "USER P5", PSP_SMEM_Low, mem_size, NULL);
	if(mem_handle < 0)
	{
		pspDebugScreenPrintf("\n\nsceKernelAllocPartitionMemory: error ( %#08x )\n", mem_handle);

		pspDebugScreenPrintf("press START to exit\n");
		do
		{
			sceCtrlReadBufferPositive(&pad_data, 1);
		}
		while(!(pad_data.Buttons & PSP_CTRL_START));

		sceKernelVolatileMemUnlock(0);
		sceKernelExitGame();

		return 0;
	}

	mem_ptr = (unsigned char*)sceKernelGetBlockHeadAddr(mem_handle);
	if(!P5_CHECK(mem_ptr))
	{
		pspDebugScreenPrintf("\n\nsceKernelAllocPartitionMemory: %#08x not in range %#08x - %#08x\n",
			(unsigned int)mem_ptr, (unsigned int)vol_addr, (unsigned int)vol_addr + vol_size);

		pspDebugScreenPrintf("press START to exit\n");
		do
		{
			sceCtrlReadBufferPositive(&pad_data, 1);
		}
		while(!(pad_data.Buttons & PSP_CTRL_START));

		sceKernelFreePartitionMemory(mem_handle);
		sceKernelVolatileMemUnlock(0);
		sceKernelExitGame();

		return 0;
	}

	pad_buttons = 0;

	pspDebugScreenPrintf("\nCROSS - write\nCIRCLE - read\nTRIANGLE - clear\nSTART - exit\n");
	while(1)
	{
		sceCtrlReadBufferPositive(&pad_data, 1);

		if(pad_data.Buttons != pad_buttons)
		{
			if(pad_data.Buttons & PSP_CTRL_CROSS)
			{
				pspDebugScreenPrintf("Mem write\n");
				for(int i = 0; i < mem_size; i += 4) // filling
					*(unsigned int*)&mem_ptr[i] = (unsigned int)&mem_ptr[i];
				pspDebugScreenPrintf("Done!\n");
			}
			if(pad_data.Buttons & PSP_CTRL_CIRCLE)
			{
				pspDebugScreenPrintf("Mem read\n");
				for(int i = 0; i < mem_size; i += 4)
				{
					if(*(unsigned int*)(&mem_ptr[i]) != (unsigned int)(&mem_ptr[i]))
					{
						pspDebugScreenPrintf("\n\nfailed at address: %#08x\n", (unsigned int)&mem_ptr[i]);
						break;
					}
				}
				pspDebugScreenPrintf("Done!\n");
			}
			if(pad_data.Buttons & PSP_CTRL_TRIANGLE)
			{
				pspDebugScreenPrintf("Mem clear\n");
				for(int i = 0; i < mem_size; i += 4) // filling
					*(unsigned int*)&mem_ptr[i] = 0;
				pspDebugScreenPrintf("Done!\n");
			}
			if(pad_data.Buttons & PSP_CTRL_START)
				break;

			pad_buttons = pad_data.Buttons;
		}
		sceDisplayWaitVblankStart();
	}

	sceKernelFreePartitionMemory(mem_handle);
	sceKernelVolatileMemUnlock(0);
	sceKernelExitGame();

	return 0;
}