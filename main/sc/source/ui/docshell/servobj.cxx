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
#include "precompiled_sc.hxx"
// System - Includes -----------------------------------------------------



#include <sot/formats.hxx>
#include <sfx2/app.hxx>
#include <sfx2/linkmgr.hxx>
#include "servobj.hxx"
#include "docsh.hxx"
#include "impex.hxx"
#include "brdcst.hxx"
#include "rangenam.hxx"
#include "sc.hrc"				// SC_HINT_AREAS_CHANGED

// -----------------------------------------------------------------------

sal_Bool lcl_FillRangeFromName( ScRange& rRange, ScDocShell* pDocSh, const String& rName )
{
	if (pDocSh)
	{
		ScDocument* pDoc = pDocSh->GetDocument();
		ScRangeName* pNames = pDoc->GetRangeName();
		if (pNames)
		{
			sal_uInt16 nPos;
			if( pNames->SearchName( rName, nPos ) )
			{
				ScRangeData* pData = (*pNames)[ nPos ];
				if ( pData->IsValidReference( rRange ) )
					return sal_True;
			}
		}
	}
	return sal_False;
}

ScServerObjectSvtListenerForwarder::ScServerObjectSvtListenerForwarder(
        ScServerObject* pObjP)
    : pObj(pObjP)
{
}

ScServerObjectSvtListenerForwarder::~ScServerObjectSvtListenerForwarder()
{
    //! do NOT access pObj
}

void ScServerObjectSvtListenerForwarder::Notify( SvtBroadcaster& /* rBC */, const SfxHint& rHint)
{
    pObj->Notify( aBroadcaster, rHint);
}

ScServerObject::ScServerObject( ScDocShell* pShell, const String& rItem ) :
    aForwarder( this ),
	pDocSh( pShell ),
	bRefreshListener( sal_False )
{
	//	parse item string

	if ( lcl_FillRangeFromName( aRange, pDocSh, rItem ) )
	{
		aItemStr = rItem;				// must be parsed again on ref update
	}
	else
	{
		//	parse ref
		ScDocument* pDoc = pDocSh->GetDocument();
		SCTAB nTab = pDocSh->GetCurTab();
		aRange.aStart.SetTab( nTab );

		if ( aRange.Parse( rItem, pDoc ) & SCA_VALID )
		{
			// area reference
		}
		else if ( aRange.aStart.Parse( rItem, pDoc, pDoc->GetAddressConvention() ) & SCA_VALID )
		{
			// cell reference
			aRange.aEnd = aRange.aStart;
		}
		else
		{
			DBG_ERROR("ScServerObject: invalid item");
		}
	}

	pDocSh->GetDocument()->GetLinkManager()->InsertServer( this );
	pDocSh->GetDocument()->StartListeningArea( aRange, &aForwarder );

	StartListening(*pDocSh);		// um mitzubekommen, wenn die DocShell geloescht wird
	StartListening(*SFX_APP());		// for SC_HINT_AREAS_CHANGED
}

__EXPORT ScServerObject::~ScServerObject()
{
	Clear();
}

void ScServerObject::Clear()
{
	if (pDocSh)
	{
		ScDocShell* pTemp = pDocSh;
		pDocSh = NULL;

		pTemp->GetDocument()->EndListeningArea( aRange, &aForwarder );
		pTemp->GetDocument()->GetLinkManager()->RemoveServer( this );
		EndListening(*pTemp);
		EndListening(*SFX_APP());
	}
}

void ScServerObject::EndListeningAll()
{
    aForwarder.EndListeningAll();
    SfxListener::EndListeningAll();
}

sal_Bool __EXPORT ScServerObject::GetData(
		::com::sun::star::uno::Any & rData /*out param*/,
        const String & rMimeType, sal_Bool /* bSynchron */ )
{
	if (!pDocSh)
		return sal_False;

	// named ranges may have changed -> update aRange
	if ( aItemStr.Len() )
	{
		ScRange aNew;
		if ( lcl_FillRangeFromName( aNew, pDocSh, aItemStr ) && aNew != aRange )
		{
			aRange = aNew;
			bRefreshListener = sal_True;
		}
	}

	if ( bRefreshListener )
	{
		//	refresh the listeners now (this is called from a timer)

		EndListeningAll();
		pDocSh->GetDocument()->StartListeningArea( aRange, &aForwarder );
		StartListening(*pDocSh);
		StartListening(*SFX_APP());
		bRefreshListener = sal_False;
	}

	String aDdeTextFmt = pDocSh->GetDdeTextFmt();
	ScDocument* pDoc = pDocSh->GetDocument();

	if( FORMAT_STRING == SotExchange::GetFormatIdFromMimeType( rMimeType ))
	{
		ScImportExport aObj( pDoc, aRange );
		if( aDdeTextFmt.GetChar(0) == 'F' )
			aObj.SetFormulas( sal_True );
		if( aDdeTextFmt.EqualsAscii( "SYLK" ) ||
			aDdeTextFmt.EqualsAscii( "FSYLK" ) )
		{
			ByteString aByteData;
			if( aObj.ExportByteString( aByteData, gsl_getSystemTextEncoding(), SOT_FORMATSTR_ID_SYLK ) )
			{
				rData <<= ::com::sun::star::uno::Sequence< sal_Int8 >(
										(sal_Int8*)aByteData.GetBuffer(),
										aByteData.Len() + 1 );
				return 1;
			}
			return 0;
		}
		if( aDdeTextFmt.EqualsAscii( "CSV" ) ||
			aDdeTextFmt.EqualsAscii( "FCSV" ) )
			aObj.SetSeparator( ',' );
        aObj.SetExportTextOptions( ScExportTextOptions( ScExportTextOptions::ToSpace, ' ', false ) );
		return aObj.ExportData( rMimeType, rData ) ? 1 : 0;
	}

	ScImportExport aObj( pDoc, aRange );
    aObj.SetExportTextOptions( ScExportTextOptions( ScExportTextOptions::ToSpace, ' ', false ) );
	if( aObj.IsRef() )
		return aObj.ExportData( rMimeType, rData ) ? 1 : 0;
	return 0;
}

void __EXPORT ScServerObject::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
	sal_Bool bDataChanged = sal_False;

	//	DocShell can't be tested via type info, because SFX_HINT_DYING comes from the dtor
	if ( &rBC == pDocSh )
	{
		//	from DocShell, only SFX_HINT_DYING is interesting
		if ( rHint.ISA(SfxSimpleHint) && ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
		{
			pDocSh = NULL;
			EndListening(*SFX_APP());
			//	don't access DocShell anymore for EndListening etc.
		}
	}
	else if (rBC.ISA(SfxApplication))
	{
		if ( aItemStr.Len() && rHint.ISA(SfxSimpleHint) &&
				((const SfxSimpleHint&)rHint).GetId() == SC_HINT_AREAS_CHANGED )
		{
			//	check if named range was modified
			ScRange aNew;
			if ( lcl_FillRangeFromName( aNew, pDocSh, aItemStr ) && aNew != aRange )
				bDataChanged = sal_True;
		}
	}
	else
	{
		//	must be from Area broadcasters

		const ScHint* pScHint = PTR_CAST( ScHint, &rHint );
		if( pScHint && (pScHint->GetId() & (SC_HINT_DATACHANGED | SC_HINT_DYING)) )
			bDataChanged = sal_True;
		else if (rHint.ISA(ScAreaChangedHint))		// position of broadcaster changed
		{
			ScRange aNewRange = ((const ScAreaChangedHint&)rHint).GetRange();
			if ( aRange != aNewRange )
			{
				bRefreshListener = sal_True;
				bDataChanged = sal_True;
			}
		}
		else if (rHint.ISA(SfxSimpleHint))
		{
            sal_uLong nId = ((const SfxSimpleHint&)rHint).GetId();
			if (nId == SFX_HINT_DYING)
			{
				//	If the range is being deleted, listening must be restarted
				//	after the deletion is complete (done in GetData)
				bRefreshListener = sal_True;
				bDataChanged = sal_True;
			}
		}
	}

	if ( bDataChanged && HasDataLinks() )
		SvLinkSource::NotifyDataChanged();
}





