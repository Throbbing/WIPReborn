#pragma once
//#include "Object.h"
#include "RefCount.h"
template<typename ReferencedType>
TRefCountPtr<ReferencedType>& TRefCountPtr<ReferencedType>::operator = (TRefCountPtr<ReferencedType>&& InPtr)
{
	if (this != &InPtr)
	{
		ReferencedType* OldReference = Reference;
		Reference = InPtr.Reference;
		InPtr.Reference = nullptr;
		if (OldReference)
		{
			OldReference->Release();
		}
	}
	return *this;
}

template<typename ReferencedType>
TRefCountPtr<ReferencedType>::TRefCountPtr(ReferencedType* InReference, bool bAddRef)
{
	Reference = InReference;
	if (Reference && bAddRef)
	{
		Reference->AddRef();
	}
}

template<typename ReferencedType>
TRefCountPtr<ReferencedType>::TRefCountPtr(const TRefCountPtr& Copy)
{
	Reference = Copy.Reference;
	if (Reference)
	{
		Reference->AddRef();
	}
}

template<typename ReferencedType>
TRefCountPtr<ReferencedType>::~TRefCountPtr()
{
	if (Reference)
	{
		Reference->Release();
	}
}

template<typename ReferencedType>
TRefCountPtr<ReferencedType>& TRefCountPtr<ReferencedType>::operator=(ReferencedType* InReference)
{
	// Call AddRef before Release, in case the new reference is the same as the old reference.
	ReferencedType* OldReference = Reference;
	Reference = InReference;
	if (Reference)
	{
		Reference->AddRef();
	}
	if (OldReference)
	{
		OldReference->Release();
	}
	return *this;
}

template<typename ReferencedType>
uint32 TRefCountPtr<ReferencedType>::GetRefCount()
{
	uint32 Result = 0;
	if (Reference)
	{
		Result = Reference->GetRefCount();
		CHECK(Result > 0); // you should never have a zero ref count if there is a live ref counted pointer (*this is live)
	}
	return Result;
}