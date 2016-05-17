/* ************************************************************************
 * Copyright 2016 Advanced Micro Devices, Inc.
 *
 * ************************************************************************ */

#pragma once
#ifndef _ROCBLAS_AUXILIARY_H_
#define _ROCBLAS_AUXILIARY_H_

#include <rocblas_types.h>

/*!\file
 * \brief rocblas_auxiliary.h provides auxilary functions in rocblas
*/



#define CHECK_HIP_ERROR(error) \
    if (error != hipSuccess) { \
      fprintf(stderr, "error: '%s'(%d) at %s:%d\n", hipGetErrorString(error), error,__FILE__, __LINE__); \
      exit(EXIT_FAILURE);\
    }


    /* ============================================================================================ */
    /*! \brief  indicates whether the pointer is on the host or device. currently HIP API can only recoginize the input ptr on deive or not
    can not recoginize it is on host or not */
    rocblas_mem_location rocblas_get_pointer_location(void *ptr);


#ifdef __cplusplus
extern "C" {
#endif


/********************************************************************************
 * \brief rocblas_handle is a structure holding the rocblas library context.
 * It must be initialized using rocblas_control_create()
 * and the returned handle must be passed
 * to all subsequent library function calls.
 * It should be destroyed at the end using rocblas_control_destroy().
 *******************************************************************************/
rocblas_status
rocblas_create_handle( rocblas_handle *handle);


/********************************************************************************
 * \brief destroy handle
 *******************************************************************************/
rocblas_status
rocblas_destroy_handle( rocblas_handle handle);


/********************************************************************************
 * \brief add stream to handle
 *******************************************************************************/
rocblas_status
rocblas_add_stream( rocblas_handle handle, hipStream_t stream );


/********************************************************************************
 * \brief remove any streams from handle, and add one
 *******************************************************************************/
rocblas_status
rocblas_set_stream( rocblas_handle handle, hipStream_t stream );


/********************************************************************************
 * \brief get stream [0] from handle
 *******************************************************************************/
rocblas_status
rocblas_get_stream( rocblas_handle handle, hipStream_t *stream );

#ifdef __cplusplus
}
#endif

#endif  /* _ROCBLAS_AUXILIARY_H_ */