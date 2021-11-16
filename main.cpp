/************************************************************
■C++11 thread
	■C++11のthreadで遊んでみる
		その1
			https://minus9d.hatenablog.com/entry/20130912/1378992525
		その2
			https://minus9d.hatenablog.com/entry/20130914/1379168684
		その3
			https://minus9d.hatenablog.com/entry/20131009/1381326432
		
	■C++ std::threadの使い方
		https://qiita.com/kurun_pan/items/f626e763e74e82a44493
	
	■cpprefjp - C++日本語リファレンス
		https://cpprefjp.github.io/reference/condition_variable/condition_variable/wait.html
		
	■haraduka´s diary
		http://haraduka.hatenadiary.jp/entry/2016/01/24/235230
		
	■C++11における同期処理(std::mutex, std::unique_guard, std::lock_guard, std::condition_variable)
		https://qiita.com/termoshtt/items/c01745ea4bcc89d37edc
		
■非同期処理とは？ 同期処理との違い、実装方法について解説
	https://www.rworks.jp/system/system-column/sys-entry/21730/
	
■std::vectorのresizeとassignの違い (C++)
	https://minus9d.hatenablog.com/entry/2021/02/07/175159

■std::deque
	https://kaworu.jpn.org/cpp/std::deque
************************************************************/

#define TEST 7

/************************************************************
************************************************************/

#if (TEST == -1)
	#include <stdio.h>
	#include <iostream>
	#include <vector>
	
	int main(){
		std::vector<int> vec1(4);
		
		printf("%d\n", vec1.size());
		
		return 0;
	}
	
#elif (TEST == 1)
	/************************************************************
	スレッドを作ってスレッドのIDを表示するだけのプログラム。
	************************************************************/
	#include <iostream>
	#include <thread>
	
	void worker() {
		std::cout << "thread id: " << std::this_thread::get_id() << std::endl;
	}
	
	int main() {
		/********************
		H/WでサポートされているCPUスレッド数.
		********************/
		std::cout << "concurrency = " << std::thread::hardware_concurrency() << std::endl; // concurrency = 並行性
		
		/********************
		********************/
		std::thread th(worker);
		
		/********************
		これをしないで実行すると、以下のErrorが出た。
			libc++abi.dylib: terminating
			zsh: abort	  ./thread
		********************/
		th.join(); // スレッド動作完了待ち -> スレッドの開放
	
		return 0;
	}
	
#elif (TEST == 2)
	/************************************************************
	スレッドに引数を渡す
	************************************************************/
	#include <iostream>
	#include <thread>
	
	void worker(int num1, int num2) {
		std::cout << num1 << std::endl;
		std::cout << num2 << std::endl;
	}
	
	int main() {
		// std::thread th(worker, 10, 100);
		std::thread th( [](int a, int b){ worker(a, b); }, 99, 199 );
		
		th.join();
	
		return 0;
	}
	
#elif (TEST == 3)
	/************************************************************
	スレッドに参照引数を渡す
	************************************************************/
	#include <iostream>
	#include <thread>
	
	void worker(int& num) {
		++num;
	}
	
	int main() {
		int num = 100;
		
		/********************
		std::ref	: 変数への参照tを保持するreference_wrapperオブジェクトを生成する
		std::cref	: const版
		********************/
		// std::thread th(worker, num);
		std::thread th(worker, std::ref(num));
		// std::thread th( [](int& a){ worker(a); }, std::ref(num) );
		
		th.join();
	
		std::cout << num << std::endl;
	
		return 0;
	}
	
#elif (TEST == 4)
	/************************************************************
	mutexで排他制御
		mtxは「使用中」を表すランプみたいなもの。
		あるスレッドがmtxの様子を見に行き、mtxのランプが消えていれば、mtxのランプを付けて、
		他のスレッドに今から自分が仕事をするので邪魔をしないでくれと表明する。
		他のスレッドは、mtxのランプが付いている間はランプが消えるまで待つしかない。
		あるスレッドが仕事を終えたら、ランプを消して、他のスレッドに席を譲る。
	************************************************************/
	#include <iostream>
	#include <thread>
	#include <vector>
	#include <mutex>
	
	std::mutex mtx;
	
	void worker() {
		/********************
		共通の資源（ここでは標準出力）を使う前にロックをかける。
		これを忘れると、あるthreadが出力している時に、他のthreadが割り込んでしまい、表示が乱れた。
			thread id: thread id: thread id: 0x70000caa7000
			thread id: 0x70000cb2a000
			0x70000cc30000
			0x70000cbad000
		********************/
		mtx.lock();
		
		// 資源(ここでは、stdout)を使った処理を行う
		std::cout << "thread id: " << std::this_thread::get_id() << std::endl;
		
		/********************
		使い終わったらロックを外す。
		lockしたのに、unlockを忘れると、他のthreadがいつまで経っても仕事ができず、固まってしまう。
		********************/
		mtx.unlock();
	}
	
	void worker_guard() {
		// lock_guardを使うと、スコープの終わりでlock()変数が破棄されるのにともなって、自動的にロックも解除される
		std::lock_guard<std::mutex> lock(mtx);
		
		std::cout << "thread id: " << std::this_thread::get_id() << std::endl;
	}
	
	int main() {
		std::vector<std::thread> ths(4);
		for (auto& th : ths) {
			th = std::thread(worker);
			// th = std::thread(worker_guard);
		}
		
		for (auto& th : ths) {
			th.join();
		}
		
		return 0;
	}
	
#elif (TEST == 5)
	/************************************************************
	condition_variable(条件変数)
		あるスレッドを待ち状態にして、ある条件が揃ったら、待ち状態にあるスレッドへ通知を行う -> 動作を再開する。と言う動作で使用。
	************************************************************/
	#include <mutex>
	#include <condition_variable>
	#include <atomic>
	#include <iostream>
	#include <thread>
	
	std::mutex mtx;
	std::condition_variable cv;
	bool is_ready = false; // for spurious wakeup
	
	void do_preparing_process(){
		{
			std::lock_guard<std::mutex> lock(mtx);
			std::cout << "Start Preparing" << std::endl;
		}
		
		std::this_thread::sleep_for(std::chrono::seconds(3));
		
		{
			std::lock_guard<std::mutex> lock(mtx);
			
			std::cout << "Finish Preparing" << std::endl;;
			is_ready = true;
			cv.notify_one();
		}
	}
	
	void do_main_process(){
		std::this_thread::sleep_for(std::chrono::seconds(1));
		
		{
			std::lock_guard<std::mutex> lock(mtx);
			
			std::cout << "Start Main Thread" << std::endl;
		}
		
		{
			std::unique_lock<std::mutex> uniq_lk(mtx); // ここでロックされる
			
			std::cout << "Wait preparation" << std::endl;
			cv.wait(uniq_lk, []{ return is_ready;});
			// 1. uniq_lk -> mtx をアンロックする
			// 2. 通知を受けるまでこのスレッドをブロックする
			// 3. 通知を受けたらuniq_lk -> mtx をロックする
		
			/********************
			諸々 処理
			ここでは"uniq_lk -> mtx"はロックされている
			********************/
			std::cout << "Finish Main Thread" << std::endl;
		} // デストラクタでアンロックする
	}
	
	int main(int argc, char const* argv[]){
		std::thread th_prepare([&]{ do_preparing_process(); });
		std::thread th_main([&]{ do_main_process(); });
		
		th_prepare.join();
		th_main.join();
		
		return 0;
	}
	
#elif (TEST == 6)
	/************************************************************
	notify_one()がwait()より先に呼ばれたらどうなるのか？
	************************************************************/
	#include <mutex>
	#include <condition_variable>
	#include <atomic>
	#include <iostream>
	#include <thread>
	
	std::mutex mtx;
	std::condition_variable cv;
	bool is_ready = false; // for spurious wakeup
	
	void do_preparing_process(){
		{
			std::lock_guard<std::mutex> lock(mtx);
			std::cout << "Start Preparing" << std::endl;
		}
		
		std::this_thread::sleep_for(std::chrono::seconds(3));
		
		{
			std::lock_guard<std::mutex> lock(mtx);
			std::cout << "Finish Preparing" << std::endl;;
			is_ready = true;
			cv.notify_one();
		}
	}
	
	void do_main_process(){
		std::this_thread::sleep_for(std::chrono::seconds(1));
		
		{
			std::lock_guard<std::mutex> lock(mtx);
			
			std::cout << "Start Main Thread" << std::endl;
			std::cout << "Doing task without Preparing..." << std::endl;	// ここで準備がいらない操作が可能
		}
		
		std::this_thread::sleep_for(std::chrono::seconds(5));
		
		{
			std::unique_lock<std::mutex> uniq_lk(mtx); // ここでロックされる
			
			std::cout << "Wait preparation" << std::endl;
			cv.wait(uniq_lk, []{ return is_ready;});
			// 1. uniq_lk -> mtx をアンロックする
			// 2. 通知を受けるまでこのスレッドをブロックする
			// 3. 通知を受けたらuniq_lk -> mtx をロックする
		
			/********************
			諸々 処理
			ここでは"uniq_lk -> mtx"はロックされている
			********************/
			std::cout << "Finish Main Thread" << std::endl;
		} // デストラクタでアンロックする
	}
	
	int main(int argc, char const* argv[]){
		std::thread th_prepare([&]{ do_preparing_process(); });
		std::thread th_main([&]{ do_main_process(); });
		
		th_prepare.join();
		th_main.join();
		
		return 0;
	}

#elif (TEST == 7)
	/************************************************************
	thread生成, notify_one, notify_all のtiming
	************************************************************/
	#include <thread>
	#include <condition_variable>
	#include <chrono>
	#include <iostream>
	
	std::mutex mtx;
	std::condition_variable cv;
	bool is_ready[10];
	
	void print( int n )
	{
		std::unique_lock< std::mutex > lock( mtx );
		
		std::cout << "THREAD in  " << n << std::endl;
		cv.wait(lock, [=]{ return is_ready[n]; } );
		
		std::cout << "THREAD out " << n << std::endl;
	}
	
	int main()
	{
		{
			std::lock_guard<std::mutex> lock(mtx);
			for(int i = 0; i < 10; i++) { is_ready[i] = false; }
		}
		
		std::thread threads[ 10 ];
		for( int i = 0; i < 10; ++i ){
			threads[ i ] = std::thread( print, i );
			std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
		}
		
		std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
		
		{
			std::lock_guard<std::mutex> lock(mtx);
			
			std::cout << "notify_all()" << std::endl;
			for(int i = 0; i < 10; i++) { is_ready[i] = true; }
			
			// cv.notify_all();
		}
		for(int i = 0; i < 10; i++){
			{
				std::lock_guard<std::mutex> lock(mtx);
				cv.notify_one();
			}
			std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
		}
		
		for( auto& t: threads )
			t.join();
		
		std::cout << "DONE" << std::endl;
	}

#endif
