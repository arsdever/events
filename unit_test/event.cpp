#include <condition_variable>
#include <future>
#include <queue>
#include <thread>

#include <events/events.h>
#include <gtest/gtest.h>

class main_thread
{
public:
	main_thread() { }

	static void add_task(std::function<void()> func)
	{
		task_queue.push(func);
		task_available.notify_one();
	}

	static std::queue<std::function<void()>> task_queue;
	static std::condition_variable			 task_available;
	static std::mutex						 mutex;
} instance;

std::queue<std::function<void()>> main_thread::task_queue {};
std::condition_variable			  main_thread::task_available {};
std::mutex						  main_thread::mutex {};
static volatile std::atomic<bool> should_stop = false;

TEST(Events, Connect)
{
	evts::event e { "name" };
	e.add_handler([] { std::cout << "event triggered" << std::endl; })
		.also([] { std::cout << "second handler" << std::endl; })
		.also([] { should_stop = true; });
	e.invoke<evts::default_dispatcher>();
	e.invoke<evts::thread_dispatcher<main_thread>>();
}

int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	std::packaged_task<int()> test_task { [] { return RUN_ALL_TESTS(); } };
	std::future<int>		  result = test_task.get_future();
	std::thread				  test_runner { std::move(test_task) };

	while (!should_stop)
		{
			if (main_thread::task_queue.empty())
				{
					std::unique_lock<std::mutex> lock(main_thread::mutex);
					main_thread::task_available.wait(lock);
				}

			std::function<void()> task = main_thread::task_queue.front();
			main_thread::task_queue.pop();
			task();
		}
	test_runner.join();
	return result.get();
}