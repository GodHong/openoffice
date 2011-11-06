/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <canvas/canvastools.hxx>

#include "attributemap.hxx"
#include "tools.hxx"


namespace slideshow
{
    namespace internal
    {
        typedef ::canvas::tools::ValueMap< AttributeType > AnimateAttributeMap;

        AttributeType mapAttributeName( const ::rtl::OUString& rAttrName )
        {
            /** Maps attribute name to AttributeType enum.

	            String entries are all case-insensitive and MUST
    	        BE STORED lowercase.

        	    String entries MUST BE SORTED in ascending order!
            */
            static AnimateAttributeMap::MapEntry lcl_attributeMap[] =
                {
                    { "charcolor", ATTRIBUTE_CHAR_COLOR },

                    { "charfontname", ATTRIBUTE_CHAR_FONT_NAME },

                    { "charheight", ATTRIBUTE_CHAR_HEIGHT },

                    { "charposture", ATTRIBUTE_CHAR_POSTURE },

                    // TODO(Q1): This should prolly be changed in PPT import
                    // { "charrotation", ATTRIBUTE_CHAR_ROTATION },
                    { "charrotation", ATTRIBUTE_ROTATE },

                    { "charunderline", ATTRIBUTE_CHAR_UNDERLINE },

                    { "charweight", ATTRIBUTE_CHAR_WEIGHT },

                    { "color", ATTRIBUTE_COLOR },

                    { "dimcolor", ATTRIBUTE_DIMCOLOR },

                    { "fillcolor", ATTRIBUTE_FILL_COLOR },

                    { "fillstyle", ATTRIBUTE_FILL_STYLE },

                    { "height", ATTRIBUTE_HEIGHT },

                    { "linecolor", ATTRIBUTE_LINE_COLOR },

                    { "linestyle", ATTRIBUTE_LINE_STYLE },

                    { "opacity", ATTRIBUTE_OPACITY },

                    { "rotate", ATTRIBUTE_ROTATE },

                    { "skewx", ATTRIBUTE_SKEW_X },

                    { "skewy", ATTRIBUTE_SKEW_Y },

                    { "visibility", ATTRIBUTE_VISIBILITY },

                    { "width", ATTRIBUTE_WIDTH },

                    { "x", ATTRIBUTE_POS_X },

                    { "y", ATTRIBUTE_POS_Y }
                };

            static AnimateAttributeMap aMap( lcl_attributeMap,
                                             sizeof(lcl_attributeMap)/sizeof(*lcl_attributeMap),
                                             false );

            AttributeType eAttributeType = ATTRIBUTE_INVALID;
            
            // determine the type from the attribute name 
            if( !aMap.lookup( rAttrName,
                              eAttributeType ) )
            {
                OSL_TRACE( "mapAttributeName(): attribute name %s not found in map.",
                           ::rtl::OUStringToOString( rAttrName, 
                                                     RTL_TEXTENCODING_ASCII_US ).getStr() );
                return ATTRIBUTE_INVALID;
            }
                
            return eAttributeType;
        }
            
    }
}
