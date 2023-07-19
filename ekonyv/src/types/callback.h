#ifndef EKONYV_CALLBACK_H
#define EKONYV_CALLBACK_H

template <typename Result, typename... Args>
class Callback {
public:
	template <typename T>
	using Ptr = Result (*)(Args..., T);

private:
	void *m_userdata;
	uint32_t m_userdata_size;
	Ptr<void *> m_handler;

public:
	uint16_t call_count;

private:
	template <typename T>
	struct DynamicDelegateData {
		Ptr<const T &> fn;
		T data;
	};

	template <typename T>
	static Result dynamicDelegateHandler(Args &&...args, void *userdata)
	{
		auto *const data = reinterpret_cast<DynamicDelegateData<T> *>(userdata);

		return data->fn(
		    static_cast<Args &&>(args)...,
		    data->data);
	}

public:
	Callback()
	    : m_userdata(nullptr), m_userdata_size(0), m_handler(nullptr), call_count(0) {}

	Callback(const Callback &other)
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

	~Callback()
	{
		clear();
	}

	void clear()
	{
		if (m_userdata_size != 0) {
			delete[] (byte *)m_userdata;
			m_userdata = nullptr;
			m_userdata_size = 0;
		}

		m_handler = nullptr;
	}

	Callback &operator=(const Callback &other)
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

	template <typename T>
	void set(Ptr<T *> fn, T *userdata)
	{
		// since sizeof(T*) == sizeof(void*), this cast is safe
		m_handler = reinterpret_cast<Ptr<void *>>(fn);
		m_userdata = userdata;
		m_userdata_size = 0;
	}

	template <typename T>
	void set(Ptr<const T &> fn, const T &userdata)
	{
		m_handler = dynamicDelegateHandler<T>;

		m_userdata = new byte[sizeof(DynamicDelegateData<T>)];
		m_userdata_size = sizeof(DynamicDelegateData<T>);

		auto *const userdata_as_delegate_data = reinterpret_cast<DynamicDelegateData<T> *>(m_userdata);
		userdata_as_delegate_data->fn = fn;
		userdata_as_delegate_data->data = userdata;
	}

	Result call(Args... args)
	{
		if (m_handler) {
			++call_count;
			return m_handler(static_cast<Args &&>(args)..., m_userdata);
		}

		return Result();
	}

	Result callOnce(Args... args)
	{
		if (m_handler && call_count == 0) {
			++call_count;
			return m_handler(static_cast<Args &&>(args)..., m_userdata);
		}

		return Result();
	}

	operator bool() const { return m_handler; }
	bool operator!() const { return !m_handler; }
};

#endif // !defined(EKONYV_CALLBACK_H)