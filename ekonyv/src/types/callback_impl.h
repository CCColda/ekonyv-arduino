/**
 * @file
 * @brief Implementation of a callback class.
 */

#ifndef EKONYV_CALLBACK_IMPL_H
#define EKONYV_CALLBACK_IMPL_H

#include "callback_def.h"

template <typename Result, typename... Args>
template <typename UserDataT>
/* static */ Result Callback<Result, Args...>::dynamicDelegateHandler(Args &&...args, void *userdata)
{
	auto *const data = reinterpret_cast<DynamicDelegateData<UserDataT> *>(userdata);

	return data->fn(
	    static_cast<Args &&>(args)...,
	    data->data);
}

template <typename Result, typename... Args>
Callback<Result, Args...>::Callback()
    : m_userdata(nullptr), m_userdata_size(0), m_handler(nullptr), call_count(0) {}

template <typename Result, typename... Args>
Callback<Result, Args...>::Callback(const Callback &other)
    : m_userdata(nullptr), m_userdata_size(other.m_userdata_size),
      m_handler(other.m_handler), call_count(other.call_count)
{
	if (m_userdata_size != 0) {
		m_userdata = new byte[m_userdata_size];
		memcpy(m_userdata, other.m_userdata, m_userdata_size);
	}
	else {
		m_userdata = other.m_userdata;
	}
}

template <typename Result, typename... Args>
Callback<Result, Args...>::~Callback()
{
	clear();
}

template <typename Result, typename... Args>
void Callback<Result, Args...>::clear()
{
	if (m_userdata_size != 0) {
		delete[] (byte *)m_userdata;
		m_userdata = nullptr;
		m_userdata_size = 0;
	}

	m_handler = nullptr;
}

template <typename Result, typename... Args>
Callback<Result, Args...> &Callback<Result, Args...>::operator=(const Callback &other)
{
	clear();

	m_userdata_size = other.m_userdata_size;
	m_handler = other.m_handler;

	if (m_userdata_size != 0) {
		m_userdata = new byte[m_userdata_size];
		memcpy(m_userdata, other.m_userdata, m_userdata_size);
	}
	else {
		m_userdata = other.m_userdata;
	}
}

template <typename Result, typename... Args>
template <typename UserDataT>
void Callback<Result, Args...>::set(Ptr<UserDataT *> fn, UserDataT *userdata)
{
	// since sizeof(UserDataT*) == sizeof(void*), this cast is safe
	m_handler = reinterpret_cast<Ptr<void *>>(fn);
	m_userdata = userdata;
	m_userdata_size = 0;
}

template <typename Result, typename... Args>
template <typename UserDataT>
void Callback<Result, Args...>::set(Ptr<const UserDataT &> fn, const UserDataT &userdata)
{
	m_handler = dynamicDelegateHandler<UserDataT>;

	m_userdata = new byte[sizeof(DynamicDelegateData<UserDataT>)];
	m_userdata_size = sizeof(DynamicDelegateData<UserDataT>);

	auto *const userdata_as_delegate_data = reinterpret_cast<DynamicDelegateData<UserDataT> *>(m_userdata);
	userdata_as_delegate_data->fn = fn;
	userdata_as_delegate_data->data = userdata;
}

template <typename Result, typename... Args>
Result Callback<Result, Args...>::call(Args... args)
{
	if (m_handler) {
		++call_count;
		return m_handler(static_cast<Args &&>(args)..., m_userdata);
	}

	return Result();
}

template <typename Result, typename... Args>
Result Callback<Result, Args...>::callOnce(Args... args)
{
	if (m_handler && call_count == 0) {
		++call_count;
		return m_handler(static_cast<Args &&>(args)..., m_userdata);
	}

	return Result();
}

#endif // !defined(EKONYV_CALLBACK_IMPL_H)