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


#ifndef __TIZEN_METADATA_EDITOR_PRIVATE_H__
#define __TIZEN_METADATA_EDITOR_PRIVATE_H__

#include <iostream>
#include <sys/stat.h>

#include <stdio.h>
#include <string.h>

#include <dlog.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>
#include <taglib/mp4file.h>
#include <taglib/flacfile.h>
#include <taglib/oggflacfile.h>
#include <taglib/vorbisfile.h>
#include <taglib/wavfile.h>
#include <taglib/id3v1tag.h>
#include <taglib/id3v2tag.h>
#include <taglib/mp4tag.h>
#include <taglib/xiphcomment.h>
#include <taglib/commentsframe.h>
#include <taglib/textidentificationframe.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/unsynchronizedlyricsframe.h>
#include <taglib/mp4item.h>
#include <taglib/mp4coverart.h>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "CAPI_MEDIA_METADATA_EDITOR"
#define META_SAFE_FREE(src)      { if (src) {free(src); src = NULL; } }
#define META_MALLOC(src, size)	{ if (size <= 0) src = NULL;  \
							else { src = (char *)malloc(size); if (src) memset(src, 0x0, size); } }

#define META_MAX_BUF_LEN 20


#define metadata_editor_debug(fmt, arg...) do { \
		LOGD(""fmt"", ##arg);     \
	} while (0)

#define metadata_editor_info(fmt, arg...) do { \
		LOGI(""fmt"", ##arg);     \
	} while (0)

#define metadata_editor_error(fmt, arg...) do { \
		LOGE(""fmt"", ##arg);     \
	} while (0)

#define metadata_editor_debug_fenter() do { \
		LOGD("<Enter>");     \
	} while (0)

#define metadata_editor_debug_fleave() do { \
		LOGD("<Leave>");     \
	} while (0)

#define metadata_editor_retvm_if(expr, val, fmt, arg...) do { \
			if (expr) { \
				LOGE(""fmt"", ##arg);	\
				return (val); \
			} \
		} while (0)

typedef struct {
	void*	file;
	int	filetype;
	bool	isOpen;
	bool	isReadOnly;
} metadata_editor_s;

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*__TIZEN_METADATA_EDITOR_PRIVATE_H__*/
