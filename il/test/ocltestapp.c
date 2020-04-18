/*****************************************************************************/
/*                                                                           */
/* Copyright (c) 2011-2015 Seoul National University.                        */
/* All rights reserved.                                                      */
/*                                                                           */
/* Redistribution and use in source and binary forms, with or without        */
/* modification, are permitted provided that the following conditions        */
/* are met:                                                                  */
/*   1. Redistributions of source code must retain the above copyright       */
/*      notice, this list of conditions and the following disclaimer.        */
/*   2. Redistributions in binary form must reproduce the above copyright    */
/*      notice, this list of conditions and the following disclaimer in the  */
/*      documentation and/or other materials provided with the distribution. */
/*   3. Neither the name of Seoul National University nor the names of its   */
/*      contributors may be used to endorse or promote products derived      */
/*      from this software without specific prior written permission.        */
/*                                                                           */
/* THIS SOFTWARE IS PROVIDED BY SEOUL NATIONAL UNIVERSITY "AS IS" AND ANY    */
/* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED */
/* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE    */
/* DISCLAIMED. IN NO EVENT SHALL SEOUL NATIONAL UNIVERSITY BE LIABLE FOR ANY */
/* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL        */
/* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS   */
/* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)     */
/* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,       */
/* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  */
/* ANY WAY OUT OF THE USE OF THIS  SOFTWARE, EVEN IF ADVISED OF THE          */
/* POSSIBILITY OF SUCH DAMAGE.                                               */
/*                                                                           */
/* Contact information:                                                      */
/*   Center for Manycore Programming                                         */
/*   Department of Computer Science and Engineering                          */
/*   Seoul National University, Seoul 08826, Korea                           */
/*   http://aces.snu.ac.kr                                                   */
/*                                                                           */
/* Contributors:                                                             */
/*   Jungwon Kim, Sangmin Seo, Gangwon Jo, Jun Lee, Jeongho Nah,             */
/*   Jungho Park, Junghyun Kim, and Jaejin Lee                               */
/*                                                                           */
/*****************************************************************************/

#include <CL/cl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_DEVICES	1024

#define CHECK_ERROR(err)                                   \
  if (err != CL_SUCCESS) {                                 \
    printf("[%s:%d] error %d\n", __FILE__, __LINE__, err); \
    exit(EXIT_FAILURE);                                    \
  }

const char* kernel_src =
    "// bitstream_1.xml\n"
    "__kernel void sample(__global int* dst, __global int* src, int offset) {\n"
    "  int id = get_global_id(0);\n"
    "  dst[id] = src[id] + offset;\n"
    "}\n";

const char* kernel_src_task = 
    "// bitstream_1.xml\n"
    "__kernel void sample(__global int* dst, __global int* src, int offset) {\n"
    "  int id = get_global_id(0);\n"
    "  int i = 0;\n"
    "  while(i++ < 10000){\n"
    "      dst[id] = src[id] + offset;\n"
    "  }\n"
    "}\n";

int main(int argc, char** argv) {
  cl_device_type    DEV_TYPE = CL_DEVICE_TYPE_ALL;
  cl_uint           num_platforms;
  cl_platform_id*   platforms;
  size_t            platform_name_size;
  char*             platform_name;
  cl_platform_id    snucl_platform = NULL;
  cl_device_id      devices[10];
  cl_device_id      device = NULL;
  cl_context_properties context_properties[3];
  cl_context        context;
  cl_command_queue  command_queue;
  cl_program        program;
  cl_kernel         kernel;
  int*              host_src;
  int*              host_dst;
  cl_mem            buffer_src;
  cl_mem            buffer_dst;
  cl_int            offset;
  size_t            local  = 128;
  size_t            global = local * 2;
  size_t            size   = global;
  int               i;
  cl_int            err;

  // KB add-ons
  cl_uint           num_devices = 0;

  printf ("KB: in main\n");

  err = clGetPlatformIDs(0, NULL, &num_platforms);
  CHECK_ERROR(err);
  printf("%u platforms are detected.\n", num_platforms);

  if (num_platforms < 1)
  {
    printf ("Error: no platforms detected.\n");
    exit(EXIT_FAILURE);
  }

  platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * num_platforms);
  err = clGetPlatformIDs(num_platforms, platforms, NULL);
  CHECK_ERROR(err);

  snucl_platform = platforms[0];

  if (snucl_platform == NULL) {
    printf("Error: no platforms could be fetched.\n");
    exit(EXIT_FAILURE);
  }

  err = clGetDeviceIDs(snucl_platform, DEV_TYPE, MAX_DEVICES, devices, &num_devices);
  CHECK_ERROR(err);

  if (num_devices < 1)
  {
    printf("Error: no devices could be fetched.\n");
    exit(EXIT_FAILURE);
  }

  device = devices[0];

  context_properties[0] = CL_CONTEXT_PLATFORM;
  context_properties[1] = (cl_context_properties)snucl_platform;
  context_properties[2] = NULL;
  context = clCreateContext(context_properties, 1, &device, NULL, NULL, &err);
  CHECK_ERROR(err);
  command_queue = clCreateCommandQueue(context, device, 0, &err);
  CHECK_ERROR(err);

  program = clCreateProgramWithSource(context, 1, (const char**)&kernel_src,
                                      NULL, &err);
  CHECK_ERROR(err);
  err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  CHECK_ERROR(err);
  kernel = clCreateKernel(program, "sample", &err);
  CHECK_ERROR(err);

  host_src = (int*)calloc(size, sizeof(int));
  host_dst = (int*)calloc(size, sizeof(int));
  for (i = 0; i < size; i++)
    host_src[i] = i * 10;

  buffer_src = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int) * size,
                              NULL, &err);
  CHECK_ERROR(err);
  buffer_dst = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int) * size,
                              NULL, &err);
  CHECK_ERROR(err);

  err = clEnqueueWriteBuffer(command_queue, buffer_src, CL_FALSE, 0,
                             sizeof(int) * size, host_src, 0, NULL, NULL);
  CHECK_ERROR(err);

  err = clEnqueueWriteBuffer(command_queue, buffer_dst, CL_FALSE, 0,
                             sizeof(int) * size, host_dst, 0, NULL, NULL);
  CHECK_ERROR(err);

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&buffer_dst);
  CHECK_ERROR(err);
  err = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&buffer_src);
  CHECK_ERROR(err);
  offset = 100;
  err = clSetKernelArg(kernel, 2, sizeof(cl_int), (void*)&offset);
  CHECK_ERROR(err);
  err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global, &local,
                               0, NULL, NULL);
  CHECK_ERROR(err);

  err = clEnqueueReadBuffer(command_queue, buffer_dst, CL_TRUE, 0,
                            sizeof(int) * size, host_dst, 0, NULL, NULL);
  CHECK_ERROR(err);

  for (i = 0; i < size; i++)
    printf("[%2d] %d\n", i, host_dst[i]);

  free(host_src);
  free(host_dst);

  clReleaseMemObject(buffer_src);
  clReleaseMemObject(buffer_dst);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseCommandQueue(command_queue);
  clReleaseContext(context);

  return 0;
}
