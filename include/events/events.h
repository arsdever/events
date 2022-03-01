#pragma once

#include <functional>
#include <list>

namespace evts
{
	template <typename T>
	concept thread_t = requires
	{
		T::add_task;
	};

	template <typename D>
	concept dispatcher_t = requires
	{
		D::dispatch;
	};

	class event;

	/**
	 * @brief A simple same-thread dispatcher, which dispatches the handlers on the same thread as where the event was
	 * invoked.
	 */
	struct default_dispatcher
	{
		static void dispatch(std::function<void()> func) { func(); }
	};

	/**
	 * @brief The main thread dispatcher. Dispatches the functions onto the main thread. For this you need to have the
	 * main_thread class providing @c dispatch() function.
	 */
	template <thread_t thread>
	struct thread_dispatcher
	{
		static void dispatch(std::function<void()> func) { thread::add_task(func); }
	};

	class event
	{
	private:
		class handler;
		using handlers_list_t = std::list<handler>;

	private:
		class handler
		{
		public:
			/**
			 * @brief Construct a new handler object.
			 *
			 * @param evt the event the handler handles
			 * @param func the executor function body of the handler
			 */
			handler(event& evt, std::function<void()> func);

			/**
			 * @brief Add a parallel handler chain.
			 *
			 * @return handler& the new chain root
			 */
			handler& also(std::function<void()> f);

			/**
			 * @brief Convert to a function.
			 *
			 * @return std::function<void()> the executor function body
			 */
			std::function<void()> executor() { return _func; }

		private:
			event&				  _event;
			std::function<void()> _func;
		};

	public:
		using handler_t = std::function<void()>;

	public:
		/**
		 * @brief Construct a new event object.
		 *
		 * @param name the name of the event
		 */
		event(std::string const& name);

		/**
		 * @brief Attach an event hander.
		 *
		 * @param func the event handling function body
		 * @return handler& the handler object
		 */
		handler& add_handler(std::function<void()> func);

		/**
		 * @brief Invoke the event on the specified dispatcher.
		 *
		 * @tparam dispatcher the object dispatching the executors
		 */
		template <dispatcher_t dispatcher = default_dispatcher>
		void invoke();

	private:
		std::list<handler> _handlers {};
	};

	event::handler::handler(event& evt, std::function<void()> func)
		: _event { evt }
		, _func { func }
	{
	}

	event::handler& event::handler::also(std::function<void()> func) { return _event.add_handler(func); }

	event::event(std::string const& name) { (void)name; }

	event::handler& event::add_handler(std::function<void()> func)
	{
		_handlers.push_back(handler { *this, func });
		return _handlers.back();
	}

	template <dispatcher_t dispatcher>
	void event::invoke()
	{
		for (auto handler : _handlers)
			{
				dispatcher::dispatch(handler.executor());
			}
	}
} // namespace evts