#pragma once

#include "CL\cl.h"
/* Convenient container for all OpenCL specific objects used in the sample
 *
 * It consists of two parts:
 *   - regular OpenCL objects which are used in almost each normal OpenCL applications
 *   - several OpenCL objects that are specific for this particular sample
 *
 * You collect all these objects in one structure for utility purposes
 * only, there is no OpenCL specific here: just to avoid global variables
 * and make passing all these arguments in functions easier.
 */
struct mOpenCL
{
    mOpenCL();
    ~mOpenCL();

    // Regular OpenCL objects:
    cl_context       context;           // hold the context handler
    cl_device_id     device;            // hold the selected device handler
    cl_command_queue commandQueue;      // hold the commands-queue handler
    cl_program       program;           // hold the program handler
    cl_kernel        updateSphere;      // hold the kernel handler
    float            platformVersion;   // hold the OpenCL platform version (default 1.2)
    float            deviceVersion;     // hold the OpenCL device version (default. 1.2)
    float            compilerVersion;   // hold the device OpenCL C version (default. 1.2)

    // Objects that are specific for algorithm implemented in this sample
    cl_mem           tx; // transate positon， 用在opengl中的定位
    cl_mem           ty;
    cl_mem           tz;
    cl_mem           r; // radius 半径
    cl_mem           vx; // velocity, 速度
    cl_mem           vy; 
    cl_mem           vz;
    cl_mem           m; // 质量

    cl_mem           ntx; // next tx, 副本，用于kernel中防止读写并发的错误
    cl_mem           nty;
    cl_mem           ntz;
    cl_mem           nvx;
    cl_mem           nvy;
    cl_mem           nvz;

    size_t           num;
};

mOpenCL::mOpenCL() :
    context(NULL),
    device(NULL),
    commandQueue(NULL),
    program(NULL),
    updateSphere(NULL),
    platformVersion(OPENCL_VERSION_1_2),
    deviceVersion(OPENCL_VERSION_1_2),
    compilerVersion(OPENCL_VERSION_1_2)
{
    tx = ty = tz = r = m = vx = vy = vz = NULL;
    num = 125;
    ntx = nty = ntz = nvx = nvy = nvz = NULL;
}

/*
 * destructor - called only once
 * Release all OpenCL objects
 * This is a regular sequence of calls to deallocate all created OpenCL resources in bootstrapOpenCL.
 *
 * You may want to call these deallocation procedures in the middle of your application execution
 * (not at the end) if you don't further need OpenCL runtime.
 * You may want to do that in order to free some memory, for example,
 * or recreate OpenCL objects with different parameters.
 *
 */
mOpenCL::~mOpenCL()
{
    cl_int err = CL_SUCCESS;
    if (updateSphere)
    {
        err = clReleaseKernel(updateSphere);
        if (CL_SUCCESS != err)
        {
            LogError("Error: clReleaseKernel returned '%s'.\n", TranslateOpenCLError(err));
        }
    }
    if (program)
    {
        err = clReleaseProgram(program);
        if (CL_SUCCESS != err)
        {
            LogError("Error: clReleaseProgram returned '%s'.\n", TranslateOpenCLError(err));
        }
    }
    if (tx)
    {
        err = clReleaseMemObject(tx);
        err = clReleaseMemObject(ty);
        err = clReleaseMemObject(tz);
        if (CL_SUCCESS != err)
        {
            LogError("Error: clReleaseMemObject returned '%s'.\n", TranslateOpenCLError(err));
        }
    }
    if (vx)
    {
        err = clReleaseMemObject(vx);
        err = clReleaseMemObject(vy);
        err = clReleaseMemObject(vz);
        if (CL_SUCCESS != err)
        {
            LogError("Error: clReleaseMemObject returned '%s'.\n", TranslateOpenCLError(err));
        }
    }
    if (m)
    {
        err = clReleaseMemObject(m);
        err = clReleaseMemObject(r);
        if (CL_SUCCESS != err)
        {
            LogError("Error: clReleaseMemObject returned '%s'.\n", TranslateOpenCLError(err));
        }
    }
    if (commandQueue)
    {
        err = clReleaseCommandQueue(commandQueue);
        if (CL_SUCCESS != err)
        {
            LogError("Error: clReleaseCommandQueue returned '%s'.\n", TranslateOpenCLError(err));
        }
    }
    if (device)
    {
        err = clReleaseDevice(device);
        if (CL_SUCCESS != err)
        {
            LogError("Error: clReleaseDevice returned '%s'.\n", TranslateOpenCLError(err));
        }
    }
    if (context)
    {
        err = clReleaseContext(context);
        if (CL_SUCCESS != err)
        {
            LogError("Error: clReleaseContext returned '%s'.\n", TranslateOpenCLError(err));
        }
    }

    /*
     * Note there is no procedure to deallocate platform
     * because it was not created at the startup,
     * but just queried from OpenCL runtime.
     */
}
