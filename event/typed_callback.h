/*
 * Copyright (c) 2010-2013 Juli Mallett. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef	EVENT_TYPED_CALLBACK_H
#define	EVENT_TYPED_CALLBACK_H

#include <common/thread/lock.h>

#include <event/callback.h>

template<typename T>
class TypedCallback : public CallbackBase {
	bool have_param_;
	T param_;
protected:
	TypedCallback(CallbackScheduler *scheduler, Lock *xlock)
	: CallbackBase(scheduler, xlock),
	  have_param_(false),
	  param_()
	{ }

public:
	virtual ~TypedCallback()
	{ }

protected:
	virtual void operator() (T) = 0;

public:
	void execute(void)
	{
		ASSERT("/typed/callback", have_param_);
		(*this)(param_);
	}

	void param(T p)
	{
		param_ = p;
		have_param_ = true;
	}

	void reset(void)
	{
		param_ = T();
		have_param_ = false;
	}
};

template<typename T, class C>
class ObjectTypedCallback : public TypedCallback<T> {
public:
	typedef void (C::*const method_t)(T);

private:
	C *const obj_;
	method_t method_;
public:
	template<typename Tm>
	ObjectTypedCallback(CallbackScheduler *scheduler, Lock *xlock, C *obj, Tm method)
	: TypedCallback<T>(scheduler, xlock),
	  obj_(obj),
	  method_(method)
	{ }

	~ObjectTypedCallback()
	{ }

private:
	void operator() (T p)
	{
		(obj_->*method_)(p);
	}
};

template<typename T, class C>
TypedCallback<T> *callback(Lock *lock, C *obj, void (C::*const method)(T))
{
	ASSERT_LOCK_OWNED("/callback/typed", lock);
	TypedCallback<T> *cb = new ObjectTypedCallback<T, C>(NULL, lock, obj, method);
	return (cb);
}

template<typename T, class C>
TypedCallback<T> *callback(CallbackScheduler *scheduler, Lock *lock, C *obj, void (C::*const method)(T))
{
	ASSERT_LOCK_OWNED("/callback/typed", lock);
	TypedCallback<T> *cb = new ObjectTypedCallback<T, C>(scheduler, lock, obj, method);
	return (cb);
}

#endif /* !EVENT_TYPED_CALLBACK_H */
