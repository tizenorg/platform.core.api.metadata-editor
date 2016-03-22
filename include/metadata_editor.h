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

#ifndef __TIZEN_METADATA_EDITOR_H__
#define __TIZEN_METADATA_EDITOR_H__


#include <metadata_editor_type.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @addtogroup CAPI_MEDIA_METADATA_EDITOR_MODULE
 * @{
 *
 * @file metadata_editor.h
 * @brief This file contains the API for metadata of several popular audio formats and related structure and enumeration. \n
 *        Description of metadata: title, album, artist, author, genre and description etc. \n
 */


/**
 * @brief Create metadata
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @remarks You must release @a metadata using metadata_editor_destroy().
 *
 * @param [in] metadata The handle to metadata
 * @return 0 on success, otherwise a negative error value
 * @retval #METADATA_EDITOR_ERROR_NONE Successful
 * @retval #METADATA_EDITOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #METADATA_EDITOR_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @see metadata_editor_destroy()
 */
int metadata_editor_create(metadata_editor_h *metadata);


/**
 * @brief Set file path to read or write metadata
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @remarks In case of accessing specific path in internal storage or external storage, you may add the privilege for accessing the path. \n
 *                   For example, if you get the specific path by using storage_get_directory(). you should add previlege http://tizen.org/privilege/mediastorage or http://tizen.org/privilege/externalstorage.
 *
 * @param [in] metadata The handle to metadata
 * @param [in] path path to read or write metadata
 * @return 0 on success, otherwise a negative error value
 * @retval #METADATA_EDITOR_ERROR_NONE Successful
 * @retval #METADATA_EDITOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #METADATA_EDITOR_ERROR_FILE_EXISTS File not exist
 * @retval #METADATA_EDITOR_ERROR_NOT_SUPPORTED unsupported file type
 * @retval #METADATA_EDITOR_ERROR_PERMISSION_DENIED Permission denied
 * @retval #METADATA_EDITOR_ERROR_OPERATION_FAILED Internal Operation Fail
 * @pre Create metadata handle by calling metadata_editor_create()
 * @see metadata_editor_create(), metadata_editor_destroy()
 */
int metadata_editor_set_path(metadata_editor_h metadata, const char *path);


/**
 * @brief Get the metadata corresponding to the attribute.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @remarks You must release @a value using @c free(). \n
 *                   If the attribute value of the metadata is empty, return value is NULL. \n
 *                   In case of accessing specific path in internal storage or external storage, you may add the privilege for accessing the path. \n
 *                   For example, if you get the specific path by using storage_get_directory(). you should add previlege http://tizen.org/privilege/mediastorage or http://tizen.org/privilege/externalstorage.
 *
 * @param [in] metadata The handle to metadata
 * @param [in] attribute key attribute name to get
 * @param [out] value The value of the attribute
 * @return 0 on success, otherwise a negative error value
 * @retval #METADATA_EDITOR_ERROR_NONE Successful
 * @retval #METADATA_EDITOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #METADATA_EDITOR_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #METADATA_EDITOR_ERROR_PERMISSION_DENIED Permission denied
 * @retval #METADATA_EDITOR_ERROR_OPERATION_FAILED Internal Operation Fail
 * @pre Set path to read or write metadata by calling metadata_editor_set_path()
 * @see metadata_editor_create(), metadata_editor_destroy()
 */
int metadata_editor_get_metadata(metadata_editor_h metadata, metadata_editor_attr_e attribute, char **value);


/**
 * @brief Set the attribute of the metadata.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @details This function set the attribute of the metadata for updating the metadata. \n
 *
 * @remarks You must release @a value using @c free(). \n
 *                  You must call metadata_editor_update_metadata() for applying to the metadata of the media file. if not, you will see the existing metadata when you call metadata_editor_get_metadata().
 *
 * @param [in] metadata The handle to metadata
 * @param [in] attribute key attribute name to get
 * @param [int] value The value of the attribute
 * @return 0 on success, otherwise a negative error value
 * @retval #METADATA_EDITOR_ERROR_NONE Successful
 * @retval #METADATA_EDITOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #METADATA_EDITOR_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #METADATA_EDITOR_ERROR_OPERATION_FAILED Internal Operation Fail
 * @pre Set path to read or write metadata by calling metadata_editor_set_path()
 * @see metadata_editor_create(), metadata_editor_update_metadata(), metadata_editor_destroy()
 */
int metadata_editor_set_metadata(metadata_editor_h metadata, metadata_editor_attr_e attribute, const char *value);


/**
 * @brief Update the modified metadata
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @details This function update the metadata in the media file that is modified by metadata_editor_set_metadata().
 *
 * @remarks In case of accessing specific path in internal storage or external storage, you may add the privilege for accessing the path. \n
 *                   For example, if you get the specific path by using storage_get_directory(). you should add previlege http://tizen.org/privilege/mediastorage or http://tizen.org/privilege/externalstorage.
 *
 * @param [in] metadata The handle to metadata
 * @return 0 on success, otherwise a negative error value
 * @retval #METADATA_EDITOR_ERROR_NONE Successful
 * @retval #METADATA_EDITOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #METADATA_EDITOR_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #METADATA_EDITOR_ERROR_PERMISSION_DENIED Permission denied
 * @retval #METADATA_EDITOR_ERROR_OPERATION_FAILED Internal Operation Fail
 * @pre Set path to read or write metadata by calling metadata_editor_set_path()
 * @see metadata_editor_create(), metadata_editor_destroy()
 */
int metadata_editor_update_metadata(metadata_editor_h metadata);

/**
 * @brief Get the picture in the media file
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @remarks You must release @a picture using @c free(). \n
 *                   In case of accessing specific path in internal storage or external storage, you may add the privilege for accessing the path. \n
 *                   For example, if you get the specific path by using storage_get_directory(). you should add previlege http://tizen.org/privilege/mediastorage or http://tizen.org/privilege/externalstorage.
 *
 * @param [in] metadata The handle to metadata
 * @param [in] index picture order
 * @param [out] picture encoded picture
 * @param [out] size encoded picture size
 * @param [out] mime_type the mime type of picture
 * @return 0 on success, otherwise a negative error value
 * @retval #METADATA_EDITOR_ERROR_NONE Successful
 * @retval #METADATA_EDITOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #METADATA_EDITOR_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #METADATA_EDITOR_ERROR_OPERATION_FAILED Internal Operation Fail
 * @retval #METADATA_EDITOR_ERROR_PERMISSION_DENIED Permission denied
 * @pre Set path to read or write metadata by calling metadata_editor_set_path()
 * @see metadata_editor_create(), metadata_editor_destroy()
 */
int metadata_editor_get_picture(metadata_editor_h metadata, int index, void **picture, int *size, char **mime_type);

/**
 * @brief Append the picture to the media file
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @remarks You must call metadata_editor_update_metadata() for applying to the metadata of the media file. if not, you will see the existing metadata when you call metadata_editor_get_metadata(). \n
 *                   Image type of the metadata supports jpeg and png. \n
 *                   In case of accessing specific path in internal storage or external storage, you may add the privilege for accessing the path. \n
 *                   For example, if you get the specific path by using storage_get_directory(). you should add previlege http://tizen.org/privilege/mediastorage or http://tizen.org/privilege/externalstorage.
 *
 * @param [in] metadata The handle to metadata
 * @param [in] picture_path The path of picture for adding to the metadata
 * @return 0 on success, otherwise a negative error value
 * @retval #METADATA_EDITOR_ERROR_NONE Successful
 * @retval #METADATA_EDITOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #METADATA_EDITOR_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #METADATA_EDITOR_ERROR_NOT_SUPPORTED unsupported file type
 * @retval #METADATA_EDITOR_ERROR_OPERATION_FAILED Internal Operation Fail
 * @retval #METADATA_EDITOR_ERROR_PERMISSION_DENIED Permission denied
 * @pre Set path to read or write metadata by calling metadata_editor_set_path()
 * @see metadata_editor_create(), metadata_editor_destroy()
 */
int metadata_editor_append_picture(metadata_editor_h metadata, const char *picture_path);

/**
 * @brief Remove artwork image from media file
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @remarks You must call metadata_editor_update_metadata() for applying to the metadata of the media file. if not, you will see the existing metadata when you call metadata_editor_get_metadata(). \n
 *
 * @param [in] metadata The handle to metadata
 * @param [in] index artwork image order
 * @return 0 on success, otherwise a negative error value
 * @retval #METADATA_EDITOR_ERROR_NONE Successful
 * @retval #METADATA_EDITOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #METADATA_EDITOR_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #METADATA_EDITOR_ERROR_OPERATION_FAILED Internal Operation Fail
 * @retval #METADATA_EDITOR_ERROR_PERMISSION_DENIED Permission denied
 * @pre Set path to read or write metadata by calling metadata_editor_set_path()
 * @see metadata_editor_create(), metadata_editor_destroy()
 */
int metadata_editor_remove_picture(metadata_editor_h metadata, int index);

/**
 * @brief Destroy metadata
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @param [in] metadata The handle to metadata
 * @return 0 on success, otherwise a negative error value
 * @retval #METADATA_EDITOR_ERROR_NONE Successful
 * @retval #METADATA_EDITOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #METADATA_EDITOR_ERROR_OPERATION_FAILED Internal Operation Fail
 * @pre Create metadata handle by calling metadata_editor_create()
 * @see metadata_editor_create()
 */
int metadata_editor_destroy(metadata_editor_h metadata);

/**
 * @}
 */


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TIZEN_METADATA_EDITOR_H__ */
