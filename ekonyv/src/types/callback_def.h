/**
 * @file
 * @brief Definition of a callback class.
 */

#ifndef EKONYV_CALLBACK_DEF_H
#define EKONYV_CALLBACK_DEF_H

template <typename Result, typename... Args>
//! @brief Defines a callback of a function which returns @c Result and takes @c Args... and an user data parameter.
class Callback {
public:
	template <typename UserDataT>
	//! @brief Type definition of the callback function
	using Ptr = Result (*)(Args..., UserDataT);

private:
	void *m_userdata;
	uint32_t m_userdata_size;
	Ptr<void *> m_handler;

public:
	//! @brief The number of times the underlying function was called.
	uint16_t call_count;

private:
	template <typename UserDataT>
	struct DynamicDelegateData {
		Ptr<const UserDataT &> fn;
		UserDataT data;
	};

	template <typename UserDataT>
	static Result dynamicDelegateHandler(Args &&...args, void *userdata);

public:
	//! @brief Instantiates an empty callback.
	Callback();

	//! @brief Copies a callback.
	Callback(const Callback &other);

	~Callback();

	//! @brief Empties the callback data, reverts to an empty callback.
	void clear();

	//! @brief Copy assigns a callback.
	Callback &operator=(const Callback &other);

	template <typename UserDataT>
	//! @brief Sets the callback function and user data.
	void set(Ptr<UserDataT *> fn, UserDataT *userdata);

	template <typename UserDataT>
	//! @brief Sets the callback function and user data.
	void set(Ptr<const UserDataT &> fn, const UserDataT &userdata);

	//! @brief Calls the underlying functions, increments the call count.
	Result call(Args... args);

	//! @brief Only calls the underlying function if it hasn't been called before.
	Result callOnce(Args... args);

	inline operator bool() const { return m_handler; }
	inline bool operator!() const { return !m_handler; }
};

#endif // !defined(EKONYV_CALLBACK_DEF_H)