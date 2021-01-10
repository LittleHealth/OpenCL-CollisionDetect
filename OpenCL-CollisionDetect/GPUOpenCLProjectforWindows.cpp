#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <memory.h>
#include <vector>
#include <string>
#include <iostream>
//for perf. counters
#include <Windows.h>
#include <GL\glut.h>
#include <gl\freeglut.h>
#include <gl\freeglut_ext.h>
#include <gl\freeglut_std.h>
#include <time.h>

#include "CL\cl.h"
#include "utils.h"
#include "config.h"
#include "mOpenCL.h"

using namespace std;

// 定义全局变量，同时也因此才将opencl和opengl的代码进行合并为一个cpp文件
float tx[N], ty[N], tz[N], r[N], vx[N], vy[N], vz[N], m[N], red[N], green[N], blue[N]; 
float ntx[N], nty[N], ntz[N],nvx[N], nvy[N], nvz[N];



mOpenCL ocl; // 创建opencl进行存储变量

/* 这些都是默认自带的函数
 * Check whether an OpenCL platform is the required platform
 * (based on the platform's name)
 */
bool CheckPreferredPlatformMatch(cl_platform_id platform, const char* preferredPlatform)
{
    size_t stringLength = 0;
    cl_int err = CL_SUCCESS;
    bool match = false;

    // In order to read the platform's name, we first read the platform's name string length (param_value is NULL).
    // The value returned in stringLength
    err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, 0, NULL, &stringLength);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetPlatformInfo() to get CL_PLATFORM_NAME length returned '%s'.\n", TranslateOpenCLError(err));
        return false;
    }

    // Now, that we know the platform's name string length, we can allocate enough space before read it
    std::vector<char> platformName(stringLength);

    // Read the platform's name string
    // The read value returned in platformName
    err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, stringLength, &platformName[0], NULL);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetplatform_ids() to get CL_PLATFORM_NAME returned %s.\n", TranslateOpenCLError(err));
        return false;
    }
    
    // Now check if the platform's name is the required one
    if (strstr(&platformName[0], preferredPlatform) != 0)
    {
        // The checked platform is the one we're looking for
        LogInfo("Platform: %s\n", &platformName[0]);
        match = true;
    }

    return match;
}

/* 这些都是默认自带的函数
 * Find and return the preferred OpenCL platform
 * In case that preferredPlatform is NULL, the ID of the first discovered platform will be returned
 */
cl_platform_id FindOpenCLPlatform(const char* preferredPlatform, cl_device_type deviceType)
{
    cl_uint numPlatforms = 0;
    cl_int err = CL_SUCCESS;

    // Get (in numPlatforms) the number of OpenCL platforms available
    // No platform ID will be return, since platforms is NULL
    err = clGetPlatformIDs(0, NULL, &numPlatforms);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetplatform_ids() to get num platforms returned %s.\n", TranslateOpenCLError(err));
        return NULL;
    }
    LogInfo("Number of available platforms: %u\n", numPlatforms);

    if (0 == numPlatforms)
    {
        LogError("Error: No platforms found!\n");
        return NULL;
    }

    std::vector<cl_platform_id> platforms(numPlatforms);

    // Now, obtains a list of numPlatforms OpenCL platforms available
    // The list of platforms available will be returned in platforms
    err = clGetPlatformIDs(numPlatforms, &platforms[0], NULL);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetplatform_ids() to get platforms returned %s.\n", TranslateOpenCLError(err));
        return NULL;
    }

    // Check if one of the available platform matches the preferred requirements
    for (cl_uint i = 0; i < numPlatforms; i++)
    {
        bool match = true;
        cl_uint numDevices = 0;

        // If the preferredPlatform is not NULL then check if platforms[i] is the required one
        // Otherwise, continue the check with platforms[i]
        if ((NULL != preferredPlatform) && (strlen(preferredPlatform) > 0))
        {
            // In case we're looking for a specific platform
            match = CheckPreferredPlatformMatch(platforms[i], preferredPlatform);
        }

        // match is true if the platform's name is the required one or don't care (NULL)
        if (match)
        {
            // Obtains the number of deviceType devices available on platform
            // When the function failed we expect numDevices to be zero.
            // We ignore the function return value since a non-zero error code
            // could happen if this platform doesn't support the specified device type.
            err = clGetDeviceIDs(platforms[i], deviceType, 0, NULL, &numDevices);
            if (CL_SUCCESS != err)
            {
                LogInfo("   Required device was not found on this platform.\n");
            }

            if (0 != numDevices)
            {
                // There is at list one device that answer the requirements
                LogInfo("   Required device was found.\n");
                return platforms[i];
            }
        }
    }

    LogError("Error: Required device was not found on any platform.\n");
    return NULL;
}


/* 这些都是默认自带的函数
 * This function read the OpenCL platdorm and device versions
 * (using clGetxxxInfo API) and stores it in the ocl structure.
 * Later it will enable us to support both OpenCL 1.2 and 2.0 platforms and devices
 * in the same program.
 */
int GetPlatformAndDeviceVersion (cl_platform_id platformId, mOpenCL *ocl)
{
    cl_int err = CL_SUCCESS;

    // Read the platform's version string length (param_value is NULL).
    // The value returned in stringLength
    size_t stringLength = 0;
    err = clGetPlatformInfo(platformId, CL_PLATFORM_VERSION, 0, NULL, &stringLength);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetPlatformInfo() to get CL_PLATFORM_VERSION length returned '%s'.\n", TranslateOpenCLError(err));
        return err;
    }

    // Now, that we know the platform's version string length, we can allocate enough space before read it
    std::vector<char> platformVersion(stringLength);

    // Read the platform's version string
    // The read value returned in platformVersion
    err = clGetPlatformInfo(platformId, CL_PLATFORM_VERSION, stringLength, &platformVersion[0], NULL);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetplatform_ids() to get CL_PLATFORM_VERSION returned %s.\n", TranslateOpenCLError(err));
        return err;
    }

    if (strstr(&platformVersion[0], "OpenCL 2.0") != NULL)
    {
        ocl->platformVersion = OPENCL_VERSION_2_0;
    }

    // Read the device's version string length (param_value is NULL).
    err = clGetDeviceInfo(ocl->device, CL_DEVICE_VERSION, 0, NULL, &stringLength);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetDeviceInfo() to get CL_DEVICE_VERSION length returned '%s'.\n", TranslateOpenCLError(err));
        return err;
    }

    // Now, that we know the device's version string length, we can allocate enough space before read it
    std::vector<char> deviceVersion(stringLength);

    // Read the device's version string
    // The read value returned in deviceVersion
    err = clGetDeviceInfo(ocl->device, CL_DEVICE_VERSION, stringLength, &deviceVersion[0], NULL);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetDeviceInfo() to get CL_DEVICE_VERSION returned %s.\n", TranslateOpenCLError(err));
        return err;
    }

    if (strstr(&deviceVersion[0], "OpenCL 2.0") != NULL)
    {
        ocl->deviceVersion = OPENCL_VERSION_2_0;
    }

    // Read the device's OpenCL C version string length (param_value is NULL).
    err = clGetDeviceInfo(ocl->device, CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &stringLength);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetDeviceInfo() to get CL_DEVICE_OPENCL_C_VERSION length returned '%s'.\n", TranslateOpenCLError(err));
        return err;
    }

    // Now, that we know the device's OpenCL C version string length, we can allocate enough space before read it
    std::vector<char> compilerVersion(stringLength);

    // Read the device's OpenCL C version string
    // The read value returned in compilerVersion
    err = clGetDeviceInfo(ocl->device, CL_DEVICE_OPENCL_C_VERSION, stringLength, &compilerVersion[0], NULL);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetDeviceInfo() to get CL_DEVICE_OPENCL_C_VERSION returned %s.\n", TranslateOpenCLError(err));
        return err;
    }

    else if (strstr(&compilerVersion[0], "OpenCL C 2.0") != NULL)
    {
        ocl->compilerVersion = OPENCL_VERSION_2_0;
    }

    cout << "PlatforVersion:" << ocl->platformVersion << "  deviceVersion:" << ocl->deviceVersion << "  CompilerVersion:" << ocl->compilerVersion << endl;
    return err;
}


/* 这些都是默认自带的函数
 * This function picks/creates necessary OpenCL objects which are needed.
 * The objects are:
 * OpenCL platform, device, context, and command queue.
 *
 * All these steps are needed to be performed once in a regular OpenCL application.
 * This happens before actual compute kernels calls are performed.
 *
 * For convenience, in this application you store all those basic OpenCL objects in structure mOpenCL,
 * so this function populates fields of this structure, which is passed as parameter ocl.
 * Please, consider reviewing the fields before going further.
 * The structure definition is right in the beginning of this file.
 */
int SetupOpenCL(mOpenCL *ocl, cl_device_type deviceType)
{
    // The following variable stores return codes for all OpenCL calls.
    cl_int err = CL_SUCCESS;

    // Query for all available OpenCL platforms on the system
    // Here you enumerate all platforms and pick one which name has preferredPlatform as a sub-string
    cl_platform_id platformId = FindOpenCLPlatform("Intel", deviceType);
    if (NULL == platformId)
    {
        LogError("Error: Failed to find OpenCL platform.\n");
        return CL_INVALID_VALUE;
    }

    // Create context with device of specified type.
    // Required device type is passed as function argument deviceType.
    // So you may use this function to create context for any CPU or GPU OpenCL device.
    // The creation is synchronized (pfn_notify is NULL) and NULL user_data
    cl_context_properties contextProperties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platformId, 0};
    ocl->context = clCreateContextFromType(contextProperties, deviceType, NULL, NULL, &err);
    if ((CL_SUCCESS != err) || (NULL == ocl->context))
    {
        LogError("Couldn't create a context, clCreateContextFromType() returned '%s'.\n", TranslateOpenCLError(err));
        return err;
    }

    // Query for OpenCL device which was used for context creation
    err = clGetContextInfo(ocl->context, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &ocl->device, NULL);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetContextInfo() to get list of devices returned %s.\n", TranslateOpenCLError(err));
        return err;
    }

    // Read the OpenCL platform's version and the device OpenCL and OpenCL C versions
    GetPlatformAndDeviceVersion(platformId, ocl);

    // Create command queue.
    // OpenCL kernels are enqueued for execution to a particular device through special objects called command queues.
    // Command queue guarantees some ordering between calls and other OpenCL commands.
    // Here you create a simple in-order OpenCL command queue that doesn't allow execution of two kernels in parallel on a target device.
#ifdef CL_VERSION_2_0
    if (OPENCL_VERSION_2_0 == ocl->deviceVersion)
    {
        const cl_command_queue_properties properties[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0};
        ocl->commandQueue = clCreateCommandQueueWithProperties(ocl->context, ocl->device, properties, &err);
    } 
    else {
        // default behavior: OpenCL 1.2
        cl_command_queue_properties properties = CL_QUEUE_PROFILING_ENABLE;
        ocl->commandQueue = clCreateCommandQueue(ocl->context, ocl->device, properties, &err);
    } 
#else
    // default behavior: OpenCL 1.2
    cl_command_queue_properties properties = CL_QUEUE_PROFILING_ENABLE;
    ocl->commandQueue = clCreateCommandQueue(ocl->context, ocl->device, properties, &err);
#endif
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateCommandQueue() returned %s.\n", TranslateOpenCLError(err));
        return err;
    }

    return CL_SUCCESS;
}


/* 
 * 从cl代码中创建cl的program
 */
int CreateAndBuildProgram(mOpenCL *ocl)
{
    cl_int err = CL_SUCCESS;

    // Upload the OpenCL C source code from the input file to source
    // The size of the C program is returned in sourceSize
    char* source = NULL;
    size_t src_size = 0;
    err = ReadSourceFromFile("Template.cl", &source, &src_size);
    if (CL_SUCCESS != err)
    {
        LogError("Error: ReadSourceFromFile returned %s.\n", TranslateOpenCLError(err));
        goto Finish;
    }

    // And now after you obtained a regular C string call clCreateProgramWithSource to create OpenCL program object.
    ocl->program = clCreateProgramWithSource(ocl->context, 1, (const char**)&source, &src_size, &err);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateProgramWithSource returned %s.\n", TranslateOpenCLError(err));
        goto Finish;
    }

    // Build the program
    // During creation a program is not built. You need to explicitly call build function.
    // Here you just use create-build sequence,
    // but there are also other possibilities when program consist of several parts,
    // some of which are libraries, and you may want to consider using clCompileProgram and clLinkProgram as
    // alternatives.
    err = clBuildProgram(ocl->program, 1, &ocl->device, "", NULL, NULL);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clBuildProgram() for source program returned %s.\n", TranslateOpenCLError(err));

        // In case of error print the build log to the standard output
        // First check the size of the log
        // Then allocate the memory and obtain the log from the program
        if (err == CL_BUILD_PROGRAM_FAILURE)
        {
            size_t log_size = 0;
            clGetProgramBuildInfo(ocl->program, ocl->device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

            std::vector<char> build_log(log_size);
            clGetProgramBuildInfo(ocl->program, ocl->device, CL_PROGRAM_BUILD_LOG, log_size, &build_log[0], NULL);

            LogError("Error happened during the build of OpenCL program.\nBuild log:%s", &build_log[0]);
        }
    }

Finish:
    if (source)
    {
        delete[] source;
        source = NULL;
    }

    return err;
}

/*
 * Create OpenCL buffers from host memory
 * These buffers will be used later by the OpenCL kernel
 */
int CreateBufferArguments(mOpenCL *ocl)
{
    cl_int err = CL_SUCCESS;
    // 创建buffer,将cpu的内存映射到gpu中，但这里使用的是use-host-ptr模型，没有进行内存的拷贝
    ocl->tx = clCreateBuffer(ocl->context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, ARRAY_SIZE * sizeof(float), &tx, &err);
    ocl->ty = clCreateBuffer(ocl->context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, ARRAY_SIZE * sizeof(float), &ty, &err); 
    ocl->tz = clCreateBuffer(ocl->context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, ARRAY_SIZE * sizeof(float), &tz, &err);
    ocl->vx = clCreateBuffer(ocl->context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, ARRAY_SIZE * sizeof(float), &vx, &err);
    ocl->vy = clCreateBuffer(ocl->context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, ARRAY_SIZE * sizeof(float), &vy, &err);
    ocl->vz = clCreateBuffer(ocl->context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, ARRAY_SIZE * sizeof(float), &vz, &err);
    ocl->r = clCreateBuffer(ocl->context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, ARRAY_SIZE * sizeof(float), &r, &err);
    ocl->m = clCreateBuffer(ocl->context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, ARRAY_SIZE * sizeof(float), &m, &err);
    ocl->ntx = clCreateBuffer(ocl->context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, ARRAY_SIZE * sizeof(float), &ntx, &err);
    ocl->nty = clCreateBuffer(ocl->context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, ARRAY_SIZE * sizeof(float), &nty, &err);
    ocl->ntz = clCreateBuffer(ocl->context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, ARRAY_SIZE * sizeof(float), &ntz, &err);
    ocl->nvx = clCreateBuffer(ocl->context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, ARRAY_SIZE * sizeof(float), &nvx, &err);
    ocl->nvy = clCreateBuffer(ocl->context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, ARRAY_SIZE * sizeof(float), &nvy, &err);
    ocl->nvz = clCreateBuffer(ocl->context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, ARRAY_SIZE * sizeof(float), &nvz, &err);
    
    // Create first image based on host memory inputA
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateImage for srcA returned %s\n", TranslateOpenCLError(err));
        return err;
    }
    return CL_SUCCESS;
}


/*
 * Set kernel arguments
 */
cl_uint SetKernelArguments(mOpenCL *ocl)
{
    // CL_SUCCESS is 0, so sum all err up, if not zero, means Error.
    cl_int err = CL_SUCCESS;
    // 设置kernel的参数
    err += clSetKernelArg(ocl->updateSphere, 0, sizeof(cl_mem), &ocl->tx);
    err += clSetKernelArg(ocl->updateSphere, 1, sizeof(cl_mem), &ocl->ty);
    err += clSetKernelArg(ocl->updateSphere, 2, sizeof(cl_mem), &ocl->tz);
    err += clSetKernelArg(ocl->updateSphere, 3, sizeof(cl_mem), &ocl->vx);
    err += clSetKernelArg(ocl->updateSphere, 4, sizeof(cl_mem), &ocl->vy);
    err += clSetKernelArg(ocl->updateSphere, 5, sizeof(cl_mem), &ocl->vz);
    err += clSetKernelArg(ocl->updateSphere, 6, sizeof(cl_mem), &ocl->r);
    err += clSetKernelArg(ocl->updateSphere, 7, sizeof(cl_mem), &ocl->m);

    err += clSetKernelArg(ocl->updateSphere, 8, sizeof(cl_mem), &ocl->ntx);
    err += clSetKernelArg(ocl->updateSphere, 9, sizeof(cl_mem), &ocl->nty);
    err += clSetKernelArg(ocl->updateSphere, 10, sizeof(cl_mem), &ocl->ntz);
    err += clSetKernelArg(ocl->updateSphere, 11, sizeof(cl_mem), &ocl->nvx);
    err += clSetKernelArg(ocl->updateSphere, 12, sizeof(cl_mem), &ocl->nvy);
    err += clSetKernelArg(ocl->updateSphere, 13, sizeof(cl_mem), &ocl->nvz);
    err += clSetKernelArg(ocl->updateSphere, 14, sizeof(int), &num);
    if (CL_SUCCESS != err)
    {
        LogError("error: Failed to set argument, returned %s\n", TranslateOpenCLError(err));
        return err;
    }
    return err;
}


/*
 * Execute the kernel
 */
cl_uint ExecuteUpdateSphereKernel(mOpenCL *ocl)
{
    cl_int err = CL_SUCCESS;
    // Define global iteration space for clEnqueueNDRangeKernel.
    size_t workSize = ARRAY_SIZE;
    // execute kernel
    err = clEnqueueNDRangeKernel(ocl->commandQueue, ocl->updateSphere, 1, NULL, &workSize, NULL, 0, NULL, NULL);
    if (CL_SUCCESS != err)
    {
        LogError("Error: Failed to run kernel, return %s\n", TranslateOpenCLError(err));
        return err;
    }
    // Wait until the queued kernel is completed by the device
    err = clFinish(ocl->commandQueue);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clFinish return %s\n", TranslateOpenCLError(err));
        return err;
    }
    return CL_SUCCESS;
}


/*
 * "Read" the result buffer (mapping the buffer to the host memory address)
 */
bool ReadAndVerify(mOpenCL *ocl)
{
    cl_int err = CL_SUCCESS;
    bool result = true;
    // read from a buffer object to host memory(tx, ty, tz, etc)
    //cout << "t:(" << tx[10] << "," << ty[10] << "," << tz[10] << ")" << " v:(" << vx[10] << "," << vy[10] << "," << vz[10] << ")" << endl;
    err = clEnqueueReadBuffer(ocl->commandQueue, ocl->ntx, CL_TRUE, 0, sizeof(float) * num, tx, 0, NULL, NULL);
    err = clEnqueueReadBuffer(ocl->commandQueue, ocl->nty, CL_TRUE, 0, sizeof(float) * num, ty, 0, NULL, NULL);
    err = clEnqueueReadBuffer(ocl->commandQueue, ocl->ntz, CL_TRUE, 0, sizeof(float) * num, tz, 0, NULL, NULL);
    err = clEnqueueReadBuffer(ocl->commandQueue, ocl->nvx, CL_TRUE, 0, sizeof(float) * num, vx, 0, NULL, NULL);
    err = clEnqueueReadBuffer(ocl->commandQueue, ocl->nvy, CL_TRUE, 0, sizeof(float) * num, vy, 0, NULL, NULL);
    err = clEnqueueReadBuffer(ocl->commandQueue, ocl->nvz, CL_TRUE, 0, sizeof(float) * num, vz, 0, NULL, NULL);
    // cout << "After t:(" << tx[10] << "," << ty[10] << "," << tz[10] << ")" << " v:(" << vx[10] << "," << vy[10] << "," << vz[10] << ")" << endl;

    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueMapBuffer returned %s\n", TranslateOpenCLError(err));
        return false;
    }

    // Call clFinish to guarantee that output region is updated
    err = clFinish(ocl->commandQueue);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clFinish returned %s\n", TranslateOpenCLError(err));
    }
    return result;
}



void reshapeWindow(GLsizei w, GLsizei h)//设定窗口大小变化的回调函数
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (GLfloat)w / (GLfloat)h, 0.1, 300);
}

void drawSolidCube(GLfloat x, GLfloat y, GLfloat z, GLfloat xl, GLfloat yl, GLfloat zl, GLubyte red, GLubyte green, GLubyte blue) {
    // 画面
    glPushMatrix();
    glColor3ub(red, green, blue);
    glTranslatef(x, y, z);
    glScalef(xl, yl, zl);
    glutSolidCube(1);
    glPopMatrix();
}

void drawSphere(GLfloat x, GLfloat y, GLfloat z, GLfloat r, GLubyte red, GLubyte green, GLubyte blue) {
    // 画球
    glPushMatrix();
    glColor3ub(red, green, blue);
    glTranslatef(x, y, z);
    glScalef(r, r, r);
    glutSolidSphere(0.99, 15, 15);
    glPopMatrix();
}

void display(void)
{
    //清除颜色和深度缓存
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearDepth(2);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //视角的变化
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(mgl_view_x, mgl_view_y, mgl_view_z, mgl_target_x, mgl_target_y, mgl_target_z, 0, 1, 0);
    glScalef(mgl_scale, mgl_scale, mgl_scale);

    drawSolidCube(0, -1, 0, 2, 0, 2, 241, 241, 241);
    drawSolidCube(0, 0, -1, 2, 2, 0, 171, 171, 171);
    drawSolidCube(-1, 0, 0, 0, 2, 2, 101, 101, 101);


    for (int i = 0; i < num; i++) {
        drawSphere(tx[i], ty[i], tz[i], r[i], red[i], green[i], blue[i]);
    }
    glutSwapBuffers();
}

void initSpheres(void)
{
    for (int i = 0; i < num; i++) {
        int index = i;
        int x_i = index % line;
        index = index / line;
        int y_i = index % line;
        index = index / line;
        int z_i = index;
        tx[i] = ntx[i] = x_i * 0.3 - 0.001 * y_i - 0.001 * z_i - 0.61;
        ty[i] = nty[i] = y_i * 0.3 - 0.001 * z_i - 0.001 * x_i - 0.61;
        tz[i] = ntz[i] = z_i * 0.3 - 0.001 * x_i - 0.001 * y_i - 0.61;
        r[i] = i % 7 == 0 ? 0.15 : 0.1;
        m[i] = i % 5 == 0 ? 2 : 1;
        red[i] = (i * COLOR) % 255;
        green[i] = ((i + 1) * COLOR) % 255;
        blue[i] = ((i + 2) * COLOR) % 255;
        if (i % 11 == 0) {
            vx[i] = nvx[i] = 0.003; vy[i] = nvy[i] = -0.01; vz[i] = nvz[i] = 0.002;
        }
        else vx[i] = vy[i] = vz[i] = nvx[i] = nvy[i] = nvz[i] = 0;
    };
}


void updateSpheres() {
    // 调用kernel函数更新
    ExecuteUpdateSphereKernel(&ocl);
    // The last part of this function: getting processed results back.
    ReadAndVerify(&ocl);
}

void timer(int id) {
    //重新计算速度位置
    updateSpheres();
    glutPostRedisplay();
    glutTimerFunc(GAP_TIME, timer, 1);
}

/*
 * main execution routine
 * Basically it consists of three parts:
 *   - generating the inputs
 *   - running OpenCL kernel
 *   - reading results of processing
 */
int _tmain(int argc, TCHAR* argv[])
{

    initSpheres();
    cl_int err;
    cl_device_type deviceType = CL_DEVICE_TYPE_GPU;
    LARGE_INTEGER perfFrequency;
    LARGE_INTEGER performanceCountNDRangeStart;
    LARGE_INTEGER performanceCountNDRangeStop;

    //initialize Open CL objects (context, queue, etc.)
    if (CL_SUCCESS != SetupOpenCL(&ocl, deviceType))
    {
        return -1;
    }

    // allocate working buffers. 
    // the buffer should be aligned with 4K page and size should fit 64-byte cached line
    //cl_uint optimizedSize = ((sizeof(cl_int) * arrayWidth * arrayHeight - 1)/64 + 1) * 64;
    //cl_int* inputA  = (cl_int*)_aligned_malloc(optimizedSize, 4096);

    // Create OpenCL buffers from host memory
    // These buffers will be used later by the OpenCL kernel
    if (CL_SUCCESS != CreateBufferArguments(&ocl))
    {
        return -1;
    }

     // Create and build the OpenCL program
    if (CL_SUCCESS != CreateAndBuildProgram(&ocl))
    {
        return -1;
    }

    // Program consists of kernels.
    // Each kernel can be called (enqueued) from the host part of OpenCL application.
    // To call the kernel, you need to create it from existing program.
    ocl.updateSphere = clCreateKernel(ocl.program, "updateSphere", &err);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateKernel returned %s\n", TranslateOpenCLError(err));
        return -1;
    }

    // Passing arguments into OpenCL kernel.
    if (CL_SUCCESS != SetKernelArguments(&ocl))
    {
        return -1;
    }
    // 以下为测试代码：
    //clock_t start, end;
    //start = clock(); 
    //for (int i = 0; i < 10000; i++) {
    //    if (i % 1000 == 0) cout << "CLProgress: " << i << "/10000" << endl;
    //    updateSpheres();
    //}
    //end = clock();   //结束时间
    //cout << "time = " << double(end - start) / CLOCKS_PER_SEC << "s" << endl;
    // Begin opengl to show the spheres and their collisions
    char** _argv = NULL;
    glutInit(&argc, _argv);
    //初始化OPENGL显示方式 
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    //设定OPENGL窗口位置和大小
    glutInitWindowSize(800, 800);
    glutInitWindowPosition(100, 100);
    //打开窗口
    glutCreateWindow("opengl3d");
    glEnable(GL_DEPTH_TEST);

    glutTimerFunc(GAP_TIME, timer, 1);
    //设定窗口大小变化的回调函数
    glutReshapeFunc(reshapeWindow);
    //开始OPENGL的循环
    glutDisplayFunc(display);
    glutMainLoop();
        return 0;
}




