/*
* Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/



#ifndef __TIZEN_METADATA_EDITOR_TYPE_H__
#define __TIZEN_METADATA_EDITOR_TYPE_H__

#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @addtogroup CAPI_MEDIA_METADATA_EDITOR_MODULE
* @{
*/

/** @brief Definition for Metadata editor Error Class */
#define METADATA_EDITOR_ERROR_CLASS				TIZEN_ERROR_METADATA_EDITOR

/**
 * @ingroup CAPI_MEDIA_METADATA_EDITOR_MODULE
 * @brief The enumerations of media metadata error
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 */
typedef enum
{
	METADATA_EDITOR_ERROR_NONE			       	= TIZEN_ERROR_NONE,				/**< Successful */
	METADATA_EDITOR_ERROR_INVALID_PARAMETER			= TIZEN_ERROR_INVALID_PARAMETER,		/**< Invalid parameter */
	METADATA_EDITOR_ERROR_OUT_OF_MEMORY 			= TIZEN_ERROR_OUT_OF_MEMORY,			/**< Out of memory */
	METADATA_EDITOR_ERROR_FILE_EXISTS			= TIZEN_ERROR_FILE_EXISTS,			/**< File not exist */
	METADATA_EDITOR_ERROR_PERMISSION_DENIED         = TIZEN_ERROR_PERMISSION_DENIED,        /**< Permission denied */
	METADATA_EDITOR_ERROR_NOT_SUPPORTED				= TIZEN_ERROR_NOT_SUPPORTED,		/**< Unsupported type */
	METADATA_EDITOR_ERROR_OPERATION_FAILED			= METADATA_EDITOR_ERROR_CLASS |0x01,		/**< Invalid internal operation */
} metadata_editor_error_e;


/**
 * @ingroup CAPI_MEDIA_METADATA_EDITOR_MODULE
 * @brief The enumerations of attribute
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 */
typedef enum {
	METADATA_EDITOR_ATTR_ARTIST,					/**< Artist*/
	METADATA_EDITOR_ATTR_TITLE,					/**< Title*/
	METADATA_EDITOR_ATTR_ALBUM,					/**< Album*/
	METADATA_EDITOR_ATTR_GENRE,					/**< Genre*/
	METADATA_EDITOR_ATTR_AUTHOR,				    /**< Author*/
	METADATA_EDITOR_ATTR_COPYRIGHT,				/**< Copyright*/
	METADATA_EDITOR_ATTR_DATE,					/**< Date*/
	METADATA_EDITOR_ATTR_DESCRIPTION,				/**< Description*/
	METADATA_EDITOR_ATTR_COMMENT,					/**< Comment*/
	METADATA_EDITOR_ATTR_TRACK_NUM,				/**< Track number info*/
	METADATA_EDITOR_ATTR_PICTURE_NUM,			/**< Picture number*/
	METADATA_EDITOR_ATTR_CONDUCTOR,				/**< Conductor*/
	METADATA_EDITOR_ATTR_UNSYNCLYRICS,			/**< Unsynchronized lyric*/
} metadata_editor_attr_e;

/**
 * @ingroup CAPI_MEDIA_METADATA_EDITOR_MODULE
 * @brief The handle of media metadata
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 */
typedef void * metadata_editor_h;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__TIZEN_METADATA_EDITOR_TYPE_H__*/
