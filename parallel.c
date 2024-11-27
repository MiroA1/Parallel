/* COMP.CE.350 Parallelization Excercise 2024
   Copyright (c) 2016 Matias Koskela matias.koskela@tut.fi
                      Heikki Kultala heikki.kultala@tut.fi
                      Topi Leppanen  topi.leppanen@tuni.fi

VERSION 1.1 - updated to not have stuck satellites so easily
VERSION 1.2 - updated to not have stuck satellites hopefully at all.
VERSION 19.0 - make all satellites affect the color with weighted average.
               add physic correctness check.
VERSION 20.0 - relax physic correctness check
VERSION 24.0 - port to SDL2
*/


#ifdef _WIN32
#include "SDL.h"
#else
#include "SDL2/SDL.h"
#endif

#include <stdio.h> // printf
#include <math.h> // INFINITY
#include <stdlib.h>
#include <string.h>

#include <CL/cl.h>

int mousePosX;
int mousePosY;

// These are used to decide the window size
//#define WINDOW_HEIGHT 1024
//#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1024
#define WINDOW_WIDTH  1920
#define SIZE WINDOW_WIDTH*WINDOW_HEIGHT

// The number of satellites can be changed to see how it affects performance.
// Benchmarks must be run with the original number of satellites
#define SATELLITE_COUNT 64

// These are used to control the satellite movement
#define SATELLITE_RADIUS 3.16f
#define MAX_VELOCITY 0.1f
#define GRAVITY 1.0f
#define DELTATIME 32
#define PHYSICSUPDATESPERFRAME 100000
#define BLACK_HOLE_RADIUS 4.5f

const int PLATFORM_INDEX = 0;
const int DEVICE_INDEX = 0;

// Stores 2D data like the coordinates
typedef struct{
   float x;
   float y;
} floatvector;

// Stores 2D data like the coordinates
typedef struct{
   double x;
   double y;
} doublevector;

// Each float may vary from 0.0f ... 1.0f
typedef struct{
   float blue;
   float green;
   float red;
} color_f32;

typedef struct {
    float blue;
    float green;
    float red;
	float reserved;
} color_f32_2;

// Stores rendered colors. Each value may vary from 0 ... 255
typedef struct{
   uint8_t blue;
   uint8_t green;
   uint8_t red;
   uint8_t reserved;
} color_u8;

// Stores the satellite data, which fly around black hole in the space
typedef struct{
   color_f32 identifier;
   floatvector position;
   floatvector velocity;
} satellite;

// Pixel buffer which is rendered to the screen
color_u8* pixels;

// Pixel buffer which is used for error checking
color_u8* correctPixels;

// Buffer for all satellites in the space
satellite* satellites;
satellite* backupSatelites;








// ## You may add your own variables here ##
const char* openclErrors[] = {
    "Success!",
    "Device not found.",
    "Device not available",
    "Compiler not available",
    "Memory object allocation failure",
    "Out of resources",
    "Out of host memory",
    "Profiling information not available",
    "Memory copy overlap",
    "Image format mismatch",
    "Image format not supported",
    "Program build failure",
    "Map failure",
    "Invalid value",
    "Invalid device type",
    "Invalid platform",
    "Invalid device",
    "Invalid context",
    "Invalid queue properties",
    "Invalid command queue",
    "Invalid host pointer",
    "Invalid memory object",
    "Invalid image format descriptor",
    "Invalid image size",
    "Invalid sampler",
    "Invalid binary",
    "Invalid build options",
    "Invalid program",
    "Invalid program executable",
    "Invalid kernel name",
    "Invalid kernel definition",
    "Invalid kernel",
    "Invalid argument index",
    "Invalid argument value",
    "Invalid argument size",
    "Invalid kernel arguments",
    "Invalid work dimension",
    "Invalid work group size",
    "Invalid work item size",
    "Invalid global offset",
    "Invalid event wait list",
    "Invalid event",
    "Invalid operation",
    "Invalid OpenGL object",
    "Invalid buffer size",
    "Invalid mip-map level",
    "Unknown",
};

const char* clErrorString(cl_int e)
{
    switch (e) {
    case CL_SUCCESS:                            return openclErrors[0];
    case CL_DEVICE_NOT_FOUND:                   return openclErrors[1];
    case CL_DEVICE_NOT_AVAILABLE:               return openclErrors[2];
    case CL_COMPILER_NOT_AVAILABLE:             return openclErrors[3];
    case CL_MEM_OBJECT_ALLOCATION_FAILURE:      return openclErrors[4];
    case CL_OUT_OF_RESOURCES:                   return openclErrors[5];
    case CL_OUT_OF_HOST_MEMORY:                 return openclErrors[6];
    case CL_PROFILING_INFO_NOT_AVAILABLE:       return openclErrors[7];
    case CL_MEM_COPY_OVERLAP:                   return openclErrors[8];
    case CL_IMAGE_FORMAT_MISMATCH:              return openclErrors[9];
    case CL_IMAGE_FORMAT_NOT_SUPPORTED:         return openclErrors[10];
    case CL_BUILD_PROGRAM_FAILURE:              return openclErrors[11];
    case CL_MAP_FAILURE:                        return openclErrors[12];
    case CL_INVALID_VALUE:                      return openclErrors[13];
    case CL_INVALID_DEVICE_TYPE:                return openclErrors[14];
    case CL_INVALID_PLATFORM:                   return openclErrors[15];
    case CL_INVALID_DEVICE:                     return openclErrors[16];
    case CL_INVALID_CONTEXT:                    return openclErrors[17];
    case CL_INVALID_QUEUE_PROPERTIES:           return openclErrors[18];
    case CL_INVALID_COMMAND_QUEUE:              return openclErrors[19];
    case CL_INVALID_HOST_PTR:                   return openclErrors[20];
    case CL_INVALID_MEM_OBJECT:                 return openclErrors[21];
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:    return openclErrors[22];
    case CL_INVALID_IMAGE_SIZE:                 return openclErrors[23];
    case CL_INVALID_SAMPLER:                    return openclErrors[24];
    case CL_INVALID_BINARY:                     return openclErrors[25];
    case CL_INVALID_BUILD_OPTIONS:              return openclErrors[26];
    case CL_INVALID_PROGRAM:                    return openclErrors[27];
    case CL_INVALID_PROGRAM_EXECUTABLE:         return openclErrors[28];
    case CL_INVALID_KERNEL_NAME:                return openclErrors[29];
    case CL_INVALID_KERNEL_DEFINITION:          return openclErrors[30];
    case CL_INVALID_KERNEL:                     return openclErrors[31];
    case CL_INVALID_ARG_INDEX:                  return openclErrors[32];
    case CL_INVALID_ARG_VALUE:                  return openclErrors[33];
    case CL_INVALID_ARG_SIZE:                   return openclErrors[34];
    case CL_INVALID_KERNEL_ARGS:                return openclErrors[35];
    case CL_INVALID_WORK_DIMENSION:             return openclErrors[36];
    case CL_INVALID_WORK_GROUP_SIZE:            return openclErrors[37];
    case CL_INVALID_WORK_ITEM_SIZE:             return openclErrors[38];
    case CL_INVALID_GLOBAL_OFFSET:              return openclErrors[39];
    case CL_INVALID_EVENT_WAIT_LIST:            return openclErrors[40];
    case CL_INVALID_EVENT:                      return openclErrors[41];
    case CL_INVALID_OPERATION:                  return openclErrors[42];
    case CL_INVALID_GL_OBJECT:                  return openclErrors[43];
    case CL_INVALID_BUFFER_SIZE:                return openclErrors[44];
    case CL_INVALID_MIP_LEVEL:                  return openclErrors[45];
    default:                                    return openclErrors[46];
    }
}
// Informational printing
void
printPlatformInfo(cl_platform_id* platformId, size_t ret_num_platforms) {
    size_t infoLength = 0;
    char* infoStr = NULL;
    cl_int status;
    for (unsigned int r = 0; r < (unsigned int)ret_num_platforms; ++r) {
        printf("Platform %d information:\n", r);
        status = clGetPlatformInfo(platformId[r], CL_PLATFORM_PROFILE, 0, NULL, &infoLength);
        if (status != CL_SUCCESS) {
            printf("Platform profile length error: %s\n", clErrorString(status));
        }
        infoStr = malloc((infoLength) * sizeof(char));
        status = clGetPlatformInfo(platformId[r], CL_PLATFORM_PROFILE, infoLength, infoStr, NULL);
        if (status != CL_SUCCESS) {
            printf("Platform profile info error: %s\n", clErrorString(status));
        }
        printf("\tProfile: %s\n", infoStr);
        free(infoStr);
        status = clGetPlatformInfo(platformId[r], CL_PLATFORM_VERSION, 0, NULL, &infoLength);
        if (status != CL_SUCCESS) {
            printf("Platform version length error: %s\n", clErrorString(status));
        }
        infoStr = malloc((infoLength) * sizeof(char));
        status = clGetPlatformInfo(platformId[r], CL_PLATFORM_VERSION, infoLength, infoStr, NULL);
        if (status != CL_SUCCESS) {
            printf("Platform version info error: %s\n", clErrorString(status));
        }
        printf("\tVersion: %s\n", infoStr);
        free(infoStr);
        status = clGetPlatformInfo(platformId[r], CL_PLATFORM_NAME, 0, NULL, &infoLength);
        if (status != CL_SUCCESS) {
            printf("Platform name length error: %s\n", clErrorString(status));
        }
        infoStr = malloc((infoLength) * sizeof(char));
        status = clGetPlatformInfo(platformId[r], CL_PLATFORM_NAME, infoLength, infoStr, NULL);
        if (status != CL_SUCCESS) {
            printf("Platform name info error: %s\n", clErrorString(status));
        }
        printf("\tName: %s\n", infoStr);
        free(infoStr);
        status = clGetPlatformInfo(platformId[r], CL_PLATFORM_VENDOR, 0, NULL, &infoLength);
        if (status != CL_SUCCESS) {
            printf("Platform vendor info length error: %s\n", clErrorString(status));
        }
        infoStr = malloc((infoLength) * sizeof(char));
        status = clGetPlatformInfo(platformId[r], CL_PLATFORM_VENDOR, infoLength, infoStr, NULL);
        if (status != CL_SUCCESS) {
            printf("Platform vendor info error: %s\n", clErrorString(status));
        }
        printf("\tVendor: %s\n", infoStr);
        free(infoStr);
        status = clGetPlatformInfo(platformId[r], CL_PLATFORM_EXTENSIONS, 0, NULL, &infoLength);
        if (status != CL_SUCCESS) {
            printf("Platform extensions info length error: %s\n", clErrorString(status));
        }
        infoStr = malloc((infoLength) * sizeof(char));
        status = clGetPlatformInfo(platformId[r], CL_PLATFORM_EXTENSIONS, infoLength, infoStr, NULL);
        if (status != CL_SUCCESS) {
            printf("Platform extensions info error: %s\n", clErrorString(status));
        }
        printf("\tExtensions: %s\n", infoStr);
        free(infoStr);
    }
    printf("\nUsing Platform %d.\n", PLATFORM_INDEX);
}

// Informational printing
void
printDeviceInfo(cl_device_id* deviceIds, size_t ret_num_devices) {
    // Print info about the devices
    size_t infoLength = 0;
    char* infoStr = NULL;
    cl_int status;

    for (unsigned int r = 0; r < ret_num_devices; ++r) {
        printf("Device %d indormation:\n", r);
        status = clGetDeviceInfo(deviceIds[r], CL_DEVICE_VENDOR, 0, NULL, &infoLength);
        if (status != CL_SUCCESS) {
            printf("Device Vendor info length error: %s\n", clErrorString(status));
        }
        infoStr = malloc((infoLength) * sizeof(char));
        status = clGetDeviceInfo(deviceIds[r], CL_DEVICE_VENDOR, infoLength, infoStr, NULL);
        if (status != CL_SUCCESS) {
            printf("Device Vendor info error: %s\n", clErrorString(status));
        }
        printf("\tVendor: %s\n", infoStr);
        free(infoStr);
        status = clGetDeviceInfo(deviceIds[r], CL_DEVICE_NAME, 0, NULL, &infoLength);
        if (status != CL_SUCCESS) {
            printf("Device name info length error: %s\n", clErrorString(status));
        }
        infoStr = malloc((infoLength) * sizeof(char));
        status = clGetDeviceInfo(deviceIds[r], CL_DEVICE_NAME, infoLength, infoStr, NULL);
        if (status != CL_SUCCESS) {
            printf("Device name info error: %s\n", clErrorString(status));
        }
        printf("\tName: %s\n", infoStr);
        free(infoStr);
        status = clGetDeviceInfo(deviceIds[r], CL_DEVICE_VERSION, 0, NULL, &infoLength);
        if (status != CL_SUCCESS) {
            printf("Device version info length error: %s\n", clErrorString(status));
        }
        infoStr = malloc((infoLength) * sizeof(char));
        status = clGetDeviceInfo(deviceIds[r], CL_DEVICE_VERSION, infoLength, infoStr, NULL);
        if (status != CL_SUCCESS) {
            printf("Device version info error: %s\n", clErrorString(status));
        }
        printf("\tVersion: %s\n", infoStr);
        free(infoStr);
    }
    printf("\nUsing Device %d.\n", DEVICE_INDEX);
}



// Global OpenCL variables


cl_context context;
cl_command_queue commandQueue;
cl_program program;
cl_kernel kernel;
cl_platform_id platform;
cl_device_id device;



// ## You may add your own initialization routines here ##


// Load the kernel source code from the file
char* loadKernelSource(char* kernelPath) {
    cl_int status;
    FILE* fp;
    char* source;
    long int size;
    printf("Program file is: %s\n", kernelPath);
    fp = fopen(kernelPath, "rb");
    if (!fp) {
        printf("Could not open kernel file\n");
        exit(-1);
    }
    status = fseek(fp, 0, SEEK_END);
    if (status != 0) {
        printf("Error seeking to end of file\n");
        exit(-1);
    }
    size = ftell(fp);
    if (size < 0) {
        printf("Error getting file position\n");
        exit(-1);
    }
    rewind(fp);
    source = (char*)malloc(size + 1);
    if (source == NULL) {
        printf("Error allocating space for the kernel source\n");
        exit(-1);
    }
    size_t readBytes = fread(source, 1, size, fp);
    if ((long int)readBytes != size) {
        printf("Error reading the kernel file\n");
        exit(-1);
    }
    source[size] = '\0';
    fclose(fp);
    return source;
}


void init(){
    
    cl_int status;

    // Get available OpenCL platforms
    cl_uint ret_num_platforms;
    status = clGetPlatformIDs(0, NULL, &ret_num_platforms);
    if (status != CL_SUCCESS) {
        printf("Error getting the number of platforms: %s", clErrorString(status));
    }
    cl_platform_id* platformId = malloc(sizeof(cl_platform_id) * ret_num_platforms);
    status = clGetPlatformIDs(ret_num_platforms, platformId, NULL);
    if (status != CL_SUCCESS) {
        printf("Error getting the platforms: %s", clErrorString(status));
    }

    // Print info about the platform
    printPlatformInfo(platformId, ret_num_platforms);

    // Get available devices
    cl_uint ret_num_devices = 0;
    status = clGetDeviceIDs(
        platformId[PLATFORM_INDEX], CL_DEVICE_TYPE_ALL, 0, NULL, &ret_num_devices);
    if (status != CL_SUCCESS) {
        printf("Error getting the number of devices: %s", clErrorString(status));
    }
    cl_device_id* deviceIds = malloc((ret_num_devices) * sizeof(cl_device_id));
    status = clGetDeviceIDs(
        platformId[PLATFORM_INDEX], CL_DEVICE_TYPE_ALL, ret_num_devices, deviceIds, &ret_num_devices);
    if (status != CL_SUCCESS) {
        printf("Error getting device ids: %s", clErrorString(status));
    }
    
	// Print info about the devices
    printDeviceInfo(deviceIds, ret_num_devices);

    // Create Context
    context = clCreateContext(NULL, 1, &(deviceIds[DEVICE_INDEX]), NULL, NULL, &status);
    if (status != CL_SUCCESS) {
        printf("Context creation error: %s\n", clErrorString(status));
    }
	printf("Context: %p\n", context);

    // Create Command Queue
    commandQueue = clCreateCommandQueue(context, deviceIds[DEVICE_INDEX], 0, &status);
    if (status != CL_SUCCESS) {
        printf("Command queue creation error: %s", clErrorString(status));
    }
	printf("Command Queue: %p\n", commandQueue);

    // Load Kernel Source
    const char* kernelSource = loadKernelSource("parallel.cl");
    if (!kernelSource) {
        printf("Error: Kernel source file not found.\n");
        exit(EXIT_FAILURE);
    }
	printf("Kernel Source: %s\n", kernelSource);

    // Create Program
    program = clCreateProgramWithSource(context, 1, &kernelSource, NULL, &status);
    if (status != CL_SUCCESS) {
        printf("Error: Failed to create program from source (Error Code: %d)\n", status);
        exit(EXIT_FAILURE);
    }
	printf("Program: %p\n", program);

    // Build Program
    status = clBuildProgram(program, 1, &deviceIds[DEVICE_INDEX], NULL, NULL, NULL);
    if (status != CL_SUCCESS) {
        printf("OpenCL build error: %s\n", clErrorString(status));
        // Fetch build errors if there were some.
        if (status == CL_BUILD_PROGRAM_FAILURE) {
            size_t infoLength = 0;
            cl_int cl_build_status = clGetProgramBuildInfo(
                program, deviceIds[DEVICE_INDEX], CL_PROGRAM_BUILD_LOG, 0, 0, &infoLength);
            if (cl_build_status != CL_SUCCESS) {
                printf("Build log length fetch error: %s\n", clErrorString(cl_build_status));
            }
            char* infoStr = malloc(infoLength * sizeof(char));
            cl_build_status = clGetProgramBuildInfo(
                program, deviceIds[DEVICE_INDEX], CL_PROGRAM_BUILD_LOG, infoLength, infoStr, 0);
            if (cl_build_status != CL_SUCCESS) {
                printf("Build log fetch error: %s\n", clErrorString(cl_build_status));
            }

            printf("OpenCL build log:\n %s", infoStr);
            free(infoStr);
        }
        abort();
    }

    // Create Kernel
    kernel = clCreateKernel(program, "parallelGraphicsEngine", &status);
    if (status != CL_SUCCESS) {
        printf("Error: Failed to create kernel (Error Code: %d)\n", status);
        exit(EXIT_FAILURE);
    }


    printf("Initialization successful!\n");

}

// ## You are asked to make this code parallel ##
// Physics engine loop. (This is called once a frame before graphics engine) 
// Moves the satellites based on gravity
// This is done multiple times in a frame because the Euler integration 
// is not accurate enough to be done only once
void parallelPhysicsEngine(){

   int tmpMousePosX = mousePosX;
   int tmpMousePosY = mousePosY;

   // double precision required for accumulation inside this routine,
   // but float storage is ok outside these loops.
   doublevector tmpPosition[SATELLITE_COUNT];
   doublevector tmpVelocity[SATELLITE_COUNT];

   int idx;
   for (idx = 0; idx < SATELLITE_COUNT; ++idx) {
       tmpPosition[idx].x = satellites[idx].position.x;
       tmpPosition[idx].y = satellites[idx].position.y;
       tmpVelocity[idx].x = satellites[idx].velocity.x;
       tmpVelocity[idx].y = satellites[idx].velocity.y;
   }
   

   
   // Physics satellite loop
   int i;
   #pragma omp parallel for
   for (i = 0; i < SATELLITE_COUNT; ++i) {
      int physicsUpdateIndex;
      // Physics iteration loop
      for (physicsUpdateIndex = 0; physicsUpdateIndex < PHYSICSUPDATESPERFRAME; ++physicsUpdateIndex) {
      

         // Distance to the blackhole (bit ugly code because C-struct cannot have member functions)
         doublevector positionToBlackHole = {.x = tmpPosition[i].x - tmpMousePosX, 
                                             .y = tmpPosition[i].y - tmpMousePosY};
         
         double distToBlackHoleSquared = positionToBlackHole.x * positionToBlackHole.x 
                                         + positionToBlackHole.y * positionToBlackHole.y;

         double distToBlackHole = sqrt(distToBlackHoleSquared);

         // Gravity force
         doublevector normalizedDirection = {.x = positionToBlackHole.x / distToBlackHole,
                                             .y = positionToBlackHole.y / distToBlackHole};

         double accumulation = GRAVITY / distToBlackHoleSquared;

         // Delta time is used to make velocity same despite different FPS
         // Update velocity based on force
         tmpVelocity[i].x -= accumulation * normalizedDirection.x *
            DELTATIME / PHYSICSUPDATESPERFRAME;
         tmpVelocity[i].y -= accumulation * normalizedDirection.y *
            DELTATIME / PHYSICSUPDATESPERFRAME;

         // Update position based on velocity
         tmpPosition[i].x +=
            tmpVelocity[i].x * DELTATIME / PHYSICSUPDATESPERFRAME;
         tmpPosition[i].y +=
            tmpVelocity[i].y * DELTATIME / PHYSICSUPDATESPERFRAME;
      }
   }

   // double precision required for accumulation inside this routine,
   // but float storage is ok outside these loops.
   // copy back the float storage.
   int idx2;
   for (idx2 = 0; idx2 < SATELLITE_COUNT; ++idx2) {
       satellites[idx2].position.x = tmpPosition[idx2].x;
       satellites[idx2].position.y = tmpPosition[idx2].y;
       satellites[idx2].velocity.x = tmpVelocity[idx2].x;
       satellites[idx2].velocity.y = tmpVelocity[idx2].y;
   }

}






void parallelGraphicsEngine() {

    cl_int status;

    int windowWidth = WINDOW_WIDTH;
    int windowHeight = WINDOW_HEIGHT;
    int satelliteCount = SATELLITE_COUNT;
	float blackHoleRadius = BLACK_HOLE_RADIUS;
    float satelliteRadius = SATELLITE_RADIUS;
	int size = SIZE;



    cl_mem pixelBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, SIZE * sizeof(color_u8), NULL, &status);
    if (status != CL_SUCCESS) {
        printf("Error: Failed to create pixelBuffer (Error Code: %d)\n", status);
        return;
    }


    floatvector* positions = malloc(sizeof(floatvector) * satelliteCount);
    color_f32_2* colors = malloc(sizeof(color_f32_2) * satelliteCount);

    for (int i = 0; i < satelliteCount; ++i) {
        positions[i] = satellites[i].position;
        positions[i] = satellites[i].position;
        colors[i].red = satellites[i].identifier.red;
		colors[i].green = satellites[i].identifier.green;
		colors[i].blue = satellites[i].identifier.blue;
        
    }


    cl_mem satellitePosBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(floatvector) * satelliteCount, NULL, &status);
    if (status != CL_SUCCESS) {
        fprintf(stderr, "OpenCL clCreateBuffer failed\n");
    }
    cl_mem satelliteColorBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(color_f32_2) * satelliteCount, NULL, &status);
    if (status != CL_SUCCESS) {
        fprintf(stderr, "OpenCL clCreateBuffer failed\n");
    }



    // Set kernel arguments
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &pixelBuffer);      // pixel buffer
    if (status != CL_SUCCESS) { printf("Error setting kernel arg 0: %d\n", status); return; }

    status = clSetKernelArg(kernel, 1, sizeof(cl_mem), &satellitePosBuffer);  // satellite position buffer
    if (status != CL_SUCCESS) { printf("Error setting kernel arg 1: %d\n", status); return; }

    status = clSetKernelArg(kernel, 2, sizeof(cl_mem), &satelliteColorBuffer); // satellite color buffer
    if (status != CL_SUCCESS) { printf("Error setting kernel arg 2: %d\n", status); return; }

    status = clSetKernelArg(kernel, 3, sizeof(int), &windowWidth);         // window width
    if (status != CL_SUCCESS) { printf("Error setting kernel arg 3: %d\n", status); return; }

    status = clSetKernelArg(kernel, 4, sizeof(int), &windowHeight);        // window height
    if (status != CL_SUCCESS) { printf("Error setting kernel arg 4: %d\n", status); return; }

    status = clSetKernelArg(kernel, 5, sizeof(int), &satelliteCount);      // satellite count
    if (status != CL_SUCCESS) { printf("Error setting kernel arg 5: %d\n", status); return; }

    status = clSetKernelArg(kernel, 6, sizeof(int), &mousePosX);          // mouse X position
    if (status != CL_SUCCESS) { printf("Error setting kernel arg 6: %d\n", status); return; }

    status = clSetKernelArg(kernel, 7, sizeof(int), &mousePosY);          // mouse Y position
    if (status != CL_SUCCESS) { printf("Error setting kernel arg 7: %d\n", status); return; }

    status = clSetKernelArg(kernel, 8, sizeof(float), &blackHoleRadius);  // black hole radius
    if (status != CL_SUCCESS) { printf("Error setting kernel arg 8: %d\n", status); return; }

    status = clSetKernelArg(kernel, 9, sizeof(float), &satelliteRadius);  // satellite radius
    if (status != CL_SUCCESS) { printf("Error setting kernel arg 9: %d\n", status); return; }



    // Enqueue the kernel
    size_t globalWorkSize[] = {WINDOW_WIDTH, WINDOW_HEIGHT};
    size_t localWorkSize[] = {16, 16};


    status = clEnqueueWriteBuffer(commandQueue, satellitePosBuffer, CL_TRUE, 0, sizeof(floatvector) * satelliteCount, positions, 0, NULL, NULL);
    if (status != CL_SUCCESS) {
        fprintf(stderr, "OpenCL clCreateBuffer failed\n");
    }

    status = clEnqueueWriteBuffer(commandQueue, satelliteColorBuffer, CL_TRUE, 0, sizeof(color_f32_2) * satelliteCount, colors, 0, NULL, NULL);
    if (status != CL_SUCCESS) {
        fprintf(stderr, "OpenCL clCreateBuffer failed\n");
    }




    // enqueue the kernel for execution
    status = clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
    if (status != CL_SUCCESS) {
        free(positions);
        free(colors);
        clReleaseMemObject(pixelBuffer);
        clReleaseMemObject(satellitePosBuffer);
        clReleaseMemObject(satelliteColorBuffer);
        printf("error: failed to enqueue kernel (error code: %d)\n", status);
        exit(EXIT_FAILURE);
        return;
    }


    // Read back the results
    status = clEnqueueReadBuffer(commandQueue, pixelBuffer, CL_TRUE, 0, SIZE * sizeof(color_u8), pixels, 0, NULL, NULL);
    if (status != CL_SUCCESS) {
        free(positions);
        free(colors);
        clReleaseMemObject(pixelBuffer);
        clReleaseMemObject(satellitePosBuffer);
        clReleaseMemObject(satelliteColorBuffer);
		printf("Error: Failed to read back pixel data (Error Code: %d)\n", status);
        exit(EXIT_FAILURE);
        return;
    }

	free(positions);
	free(colors);
	clReleaseMemObject(pixelBuffer);
	clReleaseMemObject(satellitePosBuffer);
	clReleaseMemObject(satelliteColorBuffer);


}






 /*## You are asked to make this code parallel ##
 Rendering loop (This is called once a frame after physics engine) 
 Decides the color for each pixel.*/
//void parallelGraphicsEngine() {
//
//
//    int tmpMousePosX = mousePosX;
//    int tmpMousePosY = mousePosY;
//
//    // Graphics pixel loop
//    int i;
//#pragma omp parallel for
//    for (i = 0;i < SIZE; ++i) {
//
//        // Row wise ordering
//        floatvector pixel = { .x = i % WINDOW_WIDTH, .y = i / WINDOW_WIDTH };
//
//        // Draw the black hole
//        floatvector positionToBlackHole = { .x = pixel.x -
//           tmpMousePosX, .y = pixel.y - tmpMousePosY };
//        float distToBlackHoleSquared =
//            positionToBlackHole.x * positionToBlackHole.x +
//            positionToBlackHole.y * positionToBlackHole.y;
//        float distToBlackHole = sqrt(distToBlackHoleSquared);
//        if (distToBlackHoleSquared < BLACK_HOLE_RADIUS * BLACK_HOLE_RADIUS) {
//            pixels[i].red = 0;
//            pixels[i].green = 0;
//            pixels[i].blue = 0;
//            continue; // Black hole drawing done
//        }
//
//        // This color is used for coloring the pixel
//        color_f32 renderColor = { .red = 0.f, .green = 0.f, .blue = 0.f };
//
//        // Find closest satellite
//        float shortestDistance = INFINITY;
//
//        float weights = 0.f;
//        int hitsSatellite = 0;
//
//        // First Graphics satellite loop: Find the closest satellite.
//        int j;
//        for (j = 0; j < SATELLITE_COUNT; ++j) {
//            floatvector difference = { .x = pixel.x - satellites[j].position.x,
//                                      .y = pixel.y - satellites[j].position.y };
//            float distance = sqrt(difference.x * difference.x +
//                difference.y * difference.y);
//
//            if (distance < SATELLITE_RADIUS) {
//                renderColor.red = 1.0f;
//                renderColor.green = 1.0f;
//                renderColor.blue = 1.0f;
//                hitsSatellite = 1;
//                break;
//            }
//            else {
//                float weight = 1.0f / (distance * distance * distance * distance);
//                weights += weight;
//                if (distance < shortestDistance) {
//                    shortestDistance = distance;
//                    renderColor = satellites[j].identifier;
//                }
//            }
//        }
//
//        // Second graphics loop: Calculate the color based on distance to every satellite.
//        if (!hitsSatellite) {
//            int k;
//            for (k = 0; k < SATELLITE_COUNT; ++k) {
//                floatvector difference = { .x = pixel.x - satellites[k].position.x,
//                                          .y = pixel.y - satellites[k].position.y };
//                float dist2 = (difference.x * difference.x +
//                    difference.y * difference.y);
//                float weight = 1.0f / (dist2 * dist2);
//
//                renderColor.red += (satellites[k].identifier.red *
//                    weight / weights) * 3.0f;
//
//                renderColor.green += (satellites[k].identifier.green *
//                    weight / weights) * 3.0f;
//
//                renderColor.blue += (satellites[k].identifier.blue *
//                    weight / weights) * 3.0f;
//            }
//        }
//        pixels[i].red = (uint8_t)(renderColor.red * 255.0f);
//        pixels[i].green = (uint8_t)(renderColor.green * 255.0f);
//        pixels[i].blue = (uint8_t)(renderColor.blue * 255.0f);
//    }
//
//
//}




// ## You may add your own destrcution routines here ##
void destroy(){


    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(commandQueue);
    clReleaseContext(context);

}







////////////////////////////////////////////////
// ¤¤ TO NOT EDIT ANYTHING AFTER THIS LINE ¤¤ //
////////////////////////////////////////////////

#define HORIZONTAL_CENTER (WINDOW_WIDTH / 2)
#define VERTICAL_CENTER (WINDOW_HEIGHT / 2)
SDL_Window* win;
SDL_Surface* surf;
// Is used to find out frame times
int totalTimeAcc, satelliteMovementAcc, pixelColoringAcc, frameCount;
int previousFinishTime = 0;
unsigned int frameNumber = 0;
unsigned int seed = 0;

// ¤¤ DO NOT EDIT THIS FUNCTION ¤¤
// Sequential rendering loop used for finding errors
void sequentialGraphicsEngine(){
    // Graphics pixel loop
    for(int i = 0 ;i < SIZE; ++i) {

      // Row wise ordering
      floatvector pixel = {.x = i % WINDOW_WIDTH, .y = i / WINDOW_WIDTH};

      // Draw the black hole
      floatvector positionToBlackHole = {.x = pixel.x -
         HORIZONTAL_CENTER, .y = pixel.y - VERTICAL_CENTER};
      float distToBlackHoleSquared =
         positionToBlackHole.x * positionToBlackHole.x +
         positionToBlackHole.y * positionToBlackHole.y;
      float distToBlackHole = sqrt(distToBlackHoleSquared);
      if (distToBlackHole < BLACK_HOLE_RADIUS) {
         correctPixels[i].red = 0;
         correctPixels[i].green = 0;
         correctPixels[i].blue = 0;
         continue; // Black hole drawing done
      }

      // This color is used for coloring the pixel
      color_f32 renderColor = {.red = 0.f, .green = 0.f, .blue = 0.f};

      // Find closest satellite
      float shortestDistance = INFINITY;

      float weights = 0.f;
      int hitsSatellite = 0;

      // First Graphics satellite loop: Find the closest satellite.
      for(int j = 0; j < SATELLITE_COUNT; ++j){
         floatvector difference = {.x = pixel.x - satellites[j].position.x,
                                   .y = pixel.y - satellites[j].position.y};
         float distance = sqrt(difference.x * difference.x + 
                               difference.y * difference.y);

         if(distance < SATELLITE_RADIUS) {
            renderColor.red = 1.0f;
            renderColor.green = 1.0f;
            renderColor.blue = 1.0f;
            hitsSatellite = 1;
            break;
         } else {
            float weight = 1.0f / (distance*distance*distance*distance);
            weights += weight;
            if(distance < shortestDistance){
               shortestDistance = distance;
               renderColor = satellites[j].identifier;
            }
         }
      }

      // Second graphics loop: Calculate the color based on distance to every satellite.
      if (!hitsSatellite) {
         for(int j = 0; j < SATELLITE_COUNT; ++j){
            floatvector difference = {.x = pixel.x - satellites[j].position.x,
                                      .y = pixel.y - satellites[j].position.y};
            float dist2 = (difference.x * difference.x +
                           difference.y * difference.y);
            float weight = 1.0f/(dist2* dist2);

            renderColor.red += (satellites[j].identifier.red *
                                weight /weights) * 3.0f;

            renderColor.green += (satellites[j].identifier.green *
                                  weight / weights) * 3.0f;

            renderColor.blue += (satellites[j].identifier.blue *
                                 weight / weights) * 3.0f;
         }
      }
      correctPixels[i].red = (uint8_t) (renderColor.red * 255.0f);
      correctPixels[i].green = (uint8_t) (renderColor.green * 255.0f);
      correctPixels[i].blue = (uint8_t) (renderColor.blue * 255.0f);
    }
}

void sequentialPhysicsEngine(satellite *s){

   // double precision required for accumulation inside this routine,
   // but float storage is ok outside these loops.
   doublevector tmpPosition[SATELLITE_COUNT];
   doublevector tmpVelocity[SATELLITE_COUNT];

   for (int i = 0; i < SATELLITE_COUNT; ++i) {
       tmpPosition[i].x = s[i].position.x;
       tmpPosition[i].y = s[i].position.y;
       tmpVelocity[i].x = s[i].velocity.x;
       tmpVelocity[i].y = s[i].velocity.y;
   }

   // Physics iteration loop
   for(int physicsUpdateIndex = 0;
       physicsUpdateIndex < PHYSICSUPDATESPERFRAME;
      ++physicsUpdateIndex){

       // Physics satellite loop
      for(int i = 0; i < SATELLITE_COUNT; ++i){

         // Distance to the blackhole
         // (bit ugly code because C-struct cannot have member functions)
         doublevector positionToBlackHole = {.x = tmpPosition[i].x -
            HORIZONTAL_CENTER, .y = tmpPosition[i].y - VERTICAL_CENTER};
         double distToBlackHoleSquared =
            positionToBlackHole.x * positionToBlackHole.x +
            positionToBlackHole.y * positionToBlackHole.y;
         double distToBlackHole = sqrt(distToBlackHoleSquared);

         // Gravity force
         doublevector normalizedDirection = {
            .x = positionToBlackHole.x / distToBlackHole,
            .y = positionToBlackHole.y / distToBlackHole};
         double accumulation = GRAVITY / distToBlackHoleSquared;

         // Delta time is used to make velocity same despite different FPS
         // Update velocity based on force
         tmpVelocity[i].x -= accumulation * normalizedDirection.x *
            DELTATIME / PHYSICSUPDATESPERFRAME;
         tmpVelocity[i].y -= accumulation * normalizedDirection.y *
            DELTATIME / PHYSICSUPDATESPERFRAME;

         // Update position based on velocity
         tmpPosition[i].x +=
            tmpVelocity[i].x * DELTATIME / PHYSICSUPDATESPERFRAME;
         tmpPosition[i].y +=
            tmpVelocity[i].y * DELTATIME / PHYSICSUPDATESPERFRAME;
      }
   }

   // double precision required for accumulation inside this routine,
   // but float storage is ok outside these loops.
   // copy back the float storage.
   for (int i = 0; i < SATELLITE_COUNT; ++i) {
       s[i].position.x = tmpPosition[i].x;
       s[i].position.y = tmpPosition[i].y;
       s[i].velocity.x = tmpVelocity[i].x;
       s[i].velocity.y = tmpVelocity[i].y;
   }
}

// Just some value that barely passes for OpenCL example program
#define ALLOWED_ERROR 10
#define ALLOWED_NUMBER_OF_ERRORS 10
// ¤¤ DO NOT EDIT THIS FUNCTION ¤¤
void errorCheck(){
   int countErrors = 0;
   for(unsigned int i=0; i < SIZE; ++i) {
      if(abs(correctPixels[i].red - pixels[i].red) > ALLOWED_ERROR ||
         abs(correctPixels[i].green - pixels[i].green) > ALLOWED_ERROR ||
         abs(correctPixels[i].blue - pixels[i].blue) > ALLOWED_ERROR) {
         printf("Pixel x=%d y=%d value: %d, %d, %d. Should have been: %d, %d, %d\n",
                i % WINDOW_WIDTH, i / WINDOW_WIDTH,
                pixels[i].red, pixels[i].green, pixels[i].blue,
                correctPixels[i].red, correctPixels[i].green, correctPixels[i].blue);
         countErrors++;
         if (countErrors > ALLOWED_NUMBER_OF_ERRORS) {
            printf("Too many errors (%d) in frame %d, Press enter to continue.\n", countErrors, frameNumber);
            getchar();
            return;
         }
       }
   }
   printf("Error check passed with acceptable number of wrong pixels: %d\n", countErrors);
}


// ¤¤ DO NOT EDIT THIS FUNCTION ¤¤
void compute(void){
   int timeSinceStart = SDL_GetTicks();

   // Error check during first frames
   if (frameNumber < 2) {
      memcpy(backupSatelites, satellites, sizeof(satellite) * SATELLITE_COUNT);
      sequentialPhysicsEngine(backupSatelites);
      mousePosX = HORIZONTAL_CENTER;
      mousePosY = VERTICAL_CENTER;
   } else {
      SDL_GetMouseState(&mousePosX, &mousePosY);
      if ((mousePosX == 0) && (mousePosY == 0)) {
         mousePosX = HORIZONTAL_CENTER;
         mousePosY = VERTICAL_CENTER;
      }
   }
   parallelPhysicsEngine();
   if (frameNumber < 2) {
      for (int i = 0; i < SATELLITE_COUNT; i++) {
         if (memcmp (&satellites[i], &backupSatelites[i], sizeof(satellite))) {
            printf("Incorrect satellite data of satellite: %d\n", i);
            getchar();
         }
      }
   }

   int satelliteMovementMoment = SDL_GetTicks();
   int satelliteMovementTime = satelliteMovementMoment  - timeSinceStart;

   // Decides the colors for the pixels
   parallelGraphicsEngine();

   int pixelColoringMoment = SDL_GetTicks();
   int pixelColoringTime =  pixelColoringMoment - satelliteMovementMoment;

   int finishTime = SDL_GetTicks();
   // Sequential code is used to check possible errors in the parallel version
   if(frameNumber < 2){
      sequentialGraphicsEngine();
      errorCheck();
   } else if (frameNumber == 2) {
      previousFinishTime = finishTime;
      printf("Time spent on moving satellites + Time spent on space coloring : Total time in milliseconds between frames (might not equal the sum of the left-hand expression)\n");
   } else if (frameNumber > 2) {
     // Print timings
     int totalTime = finishTime - previousFinishTime;
     previousFinishTime = finishTime;

     printf("Latency of this frame %i + %i : %ims \n",
             satelliteMovementTime, pixelColoringTime, totalTime);

     frameCount++;
     totalTimeAcc += totalTime;
     satelliteMovementAcc += satelliteMovementTime;
     pixelColoringAcc += pixelColoringTime;
     printf("Averaged over all frames: %i + %i : %ims.\n",
             satelliteMovementAcc/frameCount, pixelColoringAcc/frameCount, totalTimeAcc/frameCount);

   }
}

// ¤¤ DO NOT EDIT THIS FUNCTION ¤¤
// Probably not the best random number generator
float randomNumber(float min, float max){
   return (rand() * (max - min) / RAND_MAX) + min;
}

// DO NOT EDIT THIS FUNCTION
void fixedInit(unsigned int seed){

   if(seed != 0){
     srand(seed);
   }

   // Init pixel buffer which is rendered to the widow
   pixels = (color_u8*)malloc(sizeof(color_u8) * SIZE);

   // Init pixel buffer which is used for error checking
   correctPixels = (color_u8*)malloc(sizeof(color_u8) * SIZE);

   backupSatelites = (satellite*)malloc(sizeof(satellite) * SATELLITE_COUNT);


   // Init satellites buffer which are moving in the space
   satellites = (satellite*)malloc(sizeof(satellite) * SATELLITE_COUNT);

   // Create random satellites
   for(int i = 0; i < SATELLITE_COUNT; ++i){

      // Random reddish color
      color_f32 id = {.red = randomNumber(0.f, 0.15f) + 0.1f,
                  .green = randomNumber(0.f, 0.14f) + 0.0f,
                  .blue = randomNumber(0.f, 0.16f) + 0.0f};
    
      // Random position with margins to borders
      floatvector initialPosition = {.x = HORIZONTAL_CENTER - randomNumber(50, 320),
                              .y = VERTICAL_CENTER - randomNumber(50, 320) };
      initialPosition.x = (i / 2 % 2 == 0) ?
         initialPosition.x : WINDOW_WIDTH - initialPosition.x;
      initialPosition.y = (i < SATELLITE_COUNT / 2) ?
         initialPosition.y : WINDOW_HEIGHT - initialPosition.y;

      // Randomize velocity tangential to the balck hole
      floatvector positionToBlackHole = {.x = initialPosition.x - HORIZONTAL_CENTER,
                                    .y = initialPosition.y - VERTICAL_CENTER};
      float distance = (0.06 + randomNumber(-0.01f, 0.01f))/ 
        sqrt(positionToBlackHole.x * positionToBlackHole.x + 
          positionToBlackHole.y * positionToBlackHole.y);
      floatvector initialVelocity = {.x = distance * -positionToBlackHole.y,
                                .y = distance * positionToBlackHole.x};

      // Every other orbits clockwise
      if(i % 2 == 0){
         initialVelocity.x = -initialVelocity.x;
         initialVelocity.y = -initialVelocity.y;
      }

      satellite tmpSatelite = {.identifier = id, .position = initialPosition,
                              .velocity = initialVelocity};
      satellites[i] = tmpSatelite;
   }
}

// ¤¤ DO NOT EDIT THIS FUNCTION ¤¤
void fixedDestroy(void){
   destroy();

   free(pixels);
   free(correctPixels);
   free(satellites);

   if(seed != 0){
     printf("Used seed: %i\n", seed);
   }
}

// ¤¤ DO NOT EDIT THIS FUNCTION ¤¤
// Renders pixels-buffer to the window 
void render(void){
   SDL_LockSurface(surf);
   memcpy(surf->pixels, pixels, WINDOW_WIDTH * WINDOW_HEIGHT * 4);
   SDL_UnlockSurface(surf);

   SDL_UpdateWindowSurface(win);
   frameNumber++;
}

// DO NOT EDIT THIS FUNCTION
// Inits render window and starts mainloop
int main(int argc, char** argv){

   if(argc > 1){
     seed = atoi(argv[1]);
     printf("Using seed: %i\n", seed);
   }

   SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER);
   win = SDL_CreateWindow(
        "Satellites",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        0
    );
   surf = SDL_GetWindowSurface(win);

   fixedInit(seed);
   init();

   SDL_Event event;
   int running = 1;
   while (running) {
      while (SDL_PollEvent(&event)) switch (event.type) {
         case SDL_QUIT:
            printf("Quit called\n");
            running = 0;
            break;
      }
      compute();
      render();
   }
   SDL_Quit();
   fixedDestroy();
}
