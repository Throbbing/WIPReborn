#pragma once
#include "./RBMath/Inc/Platform/RBBasedata.h"
#include "Assertion.h"
#include <iostream>

/** A virtual interface for ref counted objects to implement. */
class IRefCountedObject
{
public:
	virtual uint32 AddRef() const = 0;
	virtual uint32 Release() const = 0;
	virtual uint32 GetRefCount() const = 0;
};


/**
* The base class of reference counted objects.
*/
class FRefCountedObject
{
public:
	FRefCountedObject() : NumRefs(0) {}
	virtual ~FRefCountedObject() { 
		CHECK(!NumRefs); 
	}
	//force ref = 1 be careful !
	void Reduce()
	{
		NumRefs = 1;
	}
	uint32 AddRef() const
	{
		//std::cout << "ref added!\n";
		return uint32(++NumRefs);
	}
	uint32 Release() const
	{
		//std::cout << "ref released!\n";
		uint32 Refs = uint32(--NumRefs);
		if (Refs == 0)
		{
			delete this;
		}
		return Refs;
	}
	uint32 GetRefCount() const
	{
		return uint32(NumRefs);
	}
private:
	mutable int32 NumRefs;
};

/**
* A smart pointer to an object which implements AddRef/Release.
*/
template<typename ReferencedType>
class TRefCountPtr
{
	typedef ReferencedType* ReferenceType;

public:

	FORCEINLINE TRefCountPtr() :
		Reference(nullptr)
	{ }

	TRefCountPtr(ReferencedType* InReference, bool bAddRef = true);

	TRefCountPtr(const TRefCountPtr& Copy);

	FORCEINLINE TRefCountPtr(TRefCountPtr&& Copy)
	{
		Reference = Copy.Reference;
		Copy.Reference = nullptr;
	}

	~TRefCountPtr();

	TRefCountPtr& operator=(ReferencedType* InReference);

	FORCEINLINE TRefCountPtr& operator=(const TRefCountPtr& InPtr)
	{
		return *this = InPtr.Reference;
	}

	TRefCountPtr& operator=(TRefCountPtr&& InPtr);

	FORCEINLINE ReferencedType* operator->() const
	{
		return Reference;
	}

	FORCEINLINE operator ReferenceType() const
	{
		return Reference;
	}

	FORCEINLINE ReferencedType** GetInitReference()
	{
		*this = nullptr;
		return &Reference;
	}

	FORCEINLINE ReferencedType* GetReference() const
	{
		return Reference;
	}

	FORCEINLINE friend bool IsValidRef(const TRefCountPtr& InReference)
	{
		return InReference.Reference != nullptr;
	}

	FORCEINLINE bool IsValid() const
	{
		return Reference != nullptr;
	}

	FORCEINLINE void SafeRelease()
	{
		*this = nullptr;
	}

	uint32 GetRefCount();

	FORCEINLINE void Swap(TRefCountPtr& InPtr) // this does not change the reference count, and so is faster
	{
		ReferencedType* OldReference = Reference;
		Reference = InPtr.Reference;
		InPtr.Reference = OldReference;
	}

	/*
	friend FArchive& operator<<(FArchive& Ar, TRefCountPtr& Ptr)
	{
	ReferenceType PtrReference = Ptr.Reference;
	Ar << PtrReference;
	if (Ar.IsLoading())
	{
	Ptr = PtrReference;
	}
	return Ar;
	}
	*/
	//
	FORCEINLINE ReferencedType* DeRef()
	{
		ReferencedType* ret = Reference;
		Reference = nullptr;
		return ret;
	}
private:

	ReferencedType* Reference;
};

template<typename ReferencedType>
FORCEINLINE bool operator==(TRefCountPtr<const ReferencedType>& A, TRefCountPtr<const ReferencedType>& B)
{
	return A.GetReference() == B.GetReference();
}

template<typename ReferencedType>
FORCEINLINE bool operator==(TRefCountPtr<const ReferencedType>& A, ReferencedType* B)
{
	return A.GetReference() == B;
}

template<typename ReferencedType>
FORCEINLINE bool operator==(ReferencedType* A, TRefCountPtr<const ReferencedType>& B)
{
	return A == B.GetReference();
}


