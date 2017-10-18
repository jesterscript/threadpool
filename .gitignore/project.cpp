#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <stdlib.h>
#include <iostream>
#include <fstream>
using namespace std;

class Iterable //Interface for iterable objects.Iterators can be used on any class derived from this.
{
public:
private:
};

//Abstract class for observer objects.
class AbstractObserver : public Iterable { 
public:
	virtual ~AbstractObserver(){}; //Deconstructor.
	virtual void Update(int _INFO) = 0; //Needed to update.
};

//Abstract class for Task and MultiTask classes.They represent tasks in the model.
class AbstractTask : public Iterable
{
public:

	virtual int getMemoryNeeded() = 0; //Accessor for required memory of a task.
	virtual bool isDone() = 0;	//is Task is done?
	virtual void assignToPool() = 0; //Assign the task into a proper thread in threadpool according to its memory.
	virtual void Action() = 0; // gonna be deleted
	virtual bool isAssigned() = 0; // Is Task assigned to a thread?
	virtual ~AbstractTask() {};
	string getName() //Accessor of name attribute.
	{
		return _name;
	}

protected:
	AbstractTask() {};
	string _name;	//Name attribute such as "Deleting browser history."
	int _totalMemoryNeeded;	// Memory required for thread for to do this task.

};

//Thread class.Represents threads in the thread pool.
class Thread : public Iterable 
{
public:
	//Constructor takes a string variable as a paramet if variable is "LThread" then
	//created thread is allocated 256 MB memory , else if it is "HThread" it alocates 512MB 
	Thread(string _TYPE)
	{
		//Initiating default values.
		_type = _TYPE; 
		_isBusy = false; 
		_currentTask = NULL; 
		
		if (_type == "HThread")
		{
			_memoryAllocated = 512; _priority = 1;
		}
		else if (_type == "LThread")
		{
			_memoryAllocated = 256; _priority = 5;
		}
	}
	void Execute() //Invokes Task's Action method.
	{
		if(_currentTask == NULL) //If the thread has no task then it is idle.
			cout << "T" << _index+1 << " is " << " IDLE"<< endl;

		else if(!_currentTask->isDone() ) // If the task is not done yet then invoke Action() of the task.
			{
				cout << "T" << _index+1 <<"("<<_memoryAllocated<<") is " ; //For console output
				_currentTask->Action(); //Perform the current task.
				
			}
		else // If task is done then empty the thread.
			{
				cout << _currentTask->getName()  << " has been completed" << endl;
				emptyTheThread();

			}
	}

	void emptyTheThread()	// Makes the thread empty.
	{
		_isBusy = false;
		_currentTask = NULL;
		
		Notify(1);	//Notifies Thread Pool about this thread is available now.Code 1 means that.

	}

	void printThread() //Print the thrad and its attributes.
	{
		cout << "T" << _index+1 << "  : " << _type << " " << _memoryAllocated << " " << _priority << endl;
	}

	bool isBusy() 
	{
		return _isBusy;
	}

	void assignTask(AbstractTask* _TASK) //Assigns current task.
	{
		_currentTask = _TASK;
		_isBusy = true;
		Notify(3);

	}
	int getMemoryAllocated() //Accessor of allocated memory of thread.
	{
		return _memoryAllocated;
	}

	AbstractTask* getTask() //Get current task of the thread.
	{
		return _currentTask;
	}

	void Attach(AbstractObserver* _OBS) //Attach observer object.
	{
		_innerObservers.push_back(_OBS);
	}

	void Detach(AbstractObserver* _OBS) // Detach observer object.
	{
	for (unsigned int i= 0; i< _innerObservers.size(); i++) {
		if (_innerObservers[i] == _OBS) {
			_innerObservers.erase(_innerObservers.begin()+i);
			return;
		}
		}	
	}

	void Notify(int _INFO) //Notifies observers about change.Observers will be TaskQueue and Memory Manager.
	{
		for(int i = 0 ; i < _innerObservers.size() ; ++i)
			_innerObservers[i]->Update(_INFO);
		//cout << "A thread notified code  : " << _INFO << endl;

	}
	void setIndex(int _INDEX) //Mutator for index of the thread.
	{
		_index = _INDEX;
	}

	int getPriority() // Accessor of priority of the thread.
	{
		return _priority;
	}
private:
	bool _isBusy; // Is thread working on a job currently?
	AbstractTask* _currentTask; //Current task the thread is performing. NULL if the thread is idle.
	int _memoryAllocated; //The thread is 256 or 512 MB allocated.
	int _priority; //If the thread is HThread then its priority is 1, else if it is LThread then 5.
	string _type;	//LThread or HThread
	int _index; // Index of the thread according the index of the thread pool's container.
	vector<AbstractObserver*> _innerObservers; //Observers of the thread.
};


class AbstractIterator {
public:
	virtual void First() = 0;
	virtual void Next() = 0;
	virtual bool IsDone() const = 0;
	virtual Iterable* CurrentItem() const = 0;
protected:
	AbstractIterator() {};
};

class ThreadPool;

//Concrete Iterator for traversing threads.Uses the thread pool as aggregate.
class ThreadIterator : public AbstractIterator {
public:
	ThreadIterator(const ThreadPool *collection);
	void First();
	void Next();
	Thread* CurrentItem() const;
	bool IsDone()const;
private:
	const ThreadPool *_collection;
	int _current;

};

class ThreadPool : public AbstractObserver //ThreadPool class with singleton pattern.An observer of a thread and also a subject of task queue.
{
public:
	ThreadIterator* CreateIterator() //Creates Thread Iterator
	{
		return new ThreadIterator(this);
	}
	void Update(int _INFO)
	{
		if(_INFO == 1) //If info is about available thread then notify the observers.
			{
				Notify(_INFO);
			}


	}
	static ThreadPool* getInstance() //Returns private unique instance .
	{
		if (_instance == NULL)
			_instance = new ThreadPool();
		return _instance;
	}

	
	int getCount() const { return _innerThreads.size(); } //Needed for iterator.
	
	Thread * get(int _CURRENT) const { return _innerThreads[_CURRENT]; } //Needed for iterator.

	Thread* getAvailableThread(int _MEMORY) // Returns proper thread according to a memory value. Used for assignToPool() in Task.
	{
		Thread* _returner = NULL;
		if(_MEMORY <= 256)
			{
				_returner = getIdleThreadByPriority(5); // if memory required to perform the task is <= 256 then getIdleThread with priority of 5.
				if(_returner == NULL) //if there is no available thread proper , then get a thread with priority 1.
					_returner = getIdleThreadByPriority(1);
			}
		else if(_MEMORY > 256)
			_returner = getIdleThreadByPriority(1);	 //if memory required to perform the task is > 256 then get idle thread with priority 1.	
		

		return _returner;
	}
	Thread* getIdleThreadByPriority(int _PRIORITY) //Returns an thread according to the specified priority value if there is a proper one.
	{
		Thread* _returner = NULL;
		
		ThreadIterator* _iterator = this->CreateIterator();
		for (_iterator->First(); !_iterator->IsDone(); _iterator->Next())
		{
			if ((*_iterator).CurrentItem()->getPriority() == _PRIORITY && !(*_iterator).CurrentItem()->isBusy())
				_returner = (*_iterator).CurrentItem(); 
		}
		delete _iterator;

		

		return _returner;
	}

	bool isThereAnyThreadWithThisValue(int _MEMORY) //This utility method is usually used for high memory required tasks.
													//As if a task has memory which can not be supported by any thread in the thread pool.
	{
		bool _answer = false;
		ThreadIterator* _iterator =	 CreateIterator();
		for (_iterator->First(); !_iterator->IsDone(); _iterator->Next())
		{
			if ((*_iterator).CurrentItem()->getMemoryAllocated() >= _MEMORY)
				_answer = true;
				
		}
		delete _iterator;
		return _answer;
	}
	void fillThePool(int _LTHREAD,int _HTHREAD) // Fills the thread pool according to desired number of HThreads and LThreads
	{
		for(int i = 0 ; i < _LTHREAD ; ++i)
			createThread("LThread");
		for(int i = 0 ; i < _HTHREAD ; ++i)
			createThread("HThread");
		for(int i  = 0 ; i < _LTHREAD + _HTHREAD ; ++i)
			Notify(1);	// According to total threads created, Notifies the thread pool for upcoming task.
		
	}

	void runAll() //Executes all threads 
	{
		
		_workingThreadCount = 0;
		ThreadIterator* _iterator = this->CreateIterator();

		for(_iterator->First(); !_iterator->IsDone(); _iterator->Next())
		{
			if((*_iterator).CurrentItem()->getTask() != NULL)
				++_workingThreadCount;
			(*_iterator).CurrentItem()->Execute();	
			
		}
		delete _iterator;
		Notify(4); // Notifies Memory Manager about execution so it can display total manager on console.

	}

	void Notify(int _INFO)  // Threadpool will notify Taskqueue to make it insert new task to new available thread.
	{
		for(int i  = 0 ; i < _innerObservers.size() ; ++i)
			_innerObservers[i]->Update(_INFO);

		//cout << "Threadpool notified code  " << _INFO << endl;

	}

	void Attach(AbstractObserver* _OBS) //Attach observer object.
	{
		_innerObservers.push_back(_OBS);
	}

	void Detach(AbstractObserver* _OBS) // Detach observer object.
	{
		for (unsigned int i= 0; i< _innerObservers.size(); i++) {
			if (_innerObservers[i] == _OBS) {
				_innerObservers.erase(_innerObservers.begin()+i);
				return;
			}
			}	
	}

	int getWorkingThreadCount()
	{
		return _workingThreadCount;
	}




private:
	ThreadPool() {_workingThreadCount = 0;};
	static ThreadPool* _instance; //Unique instance for singleton pattern.
	vector<Thread*> _innerThreads; // Threads in the thread pool.
	vector<AbstractObserver*> _innerObservers; //Observers of the thread pool.
	int _workingThreadCount;
	void createThread(string _TYPE) //Creates a thread according to specified type.
	{
		Thread* _newBornThread = new Thread(_TYPE);
		//cout << "Thread created" << endl;
		_newBornThread->Attach(this);
		_newBornThread->setIndex(_innerThreads.size());
		 _innerThreads.push_back(_newBornThread);
		 
		 
	}


	
};
ThreadPool* ThreadPool::_instance = 0;

class Task : public AbstractTask //Leaf task class.
{
public:
	Task(string _NAME,int _WORK,int _MEMORYNEED)
	{
		//Initializing values.
		_unsupportedMemory = false;
		_totalMemoryNeeded = _MEMORYNEED;
		_workToDo = _WORK;
		_name = _NAME;
		_isAssigned = false;
	}

	void Action() //Task performs.
	{
		cout << "Working on " << _name << " .. " ;
		--_workToDo;
		
	
		cout << " Routine to done : " << _workToDo << " RAM Usg : " << _totalMemoryNeeded << "MB" << endl;

	}


	void assignToPool() //Assign the task into a proper thread from the thread pool.
	{
		//Checks if memory required for task is not very high which is can not be performed.
		if(!ThreadPool::getInstance()->isThereAnyThreadWithThisValue(_totalMemoryNeeded))
			{
				_unsupportedMemory = true;
				return;
			}

 		Thread* _t1 = ThreadPool::getInstance()->getAvailableThread(this->getMemoryNeeded());//Ask for a proper thread.
 		if(_t1 != NULL && !_t1->isBusy() && !_isAssigned) //If received thread is not busy and this task is not assigned then assign this to the received thread.
 			{
 				_t1->assignTask(this);
 				_isAssigned = true;
 				//cout << _name << " is assigned to " ;
 				//_t1->printThread();
 				//cout << endl;
 			}
 		else if(_isAssigned)
 			{ //cout << _name << " is already assigned " << endl;	
 			}
 		else
 			cout << _name << " cannot be assigned.(No proper thread)" << endl;


	}
	bool isDone()  //If the task is done or memory for performing it is very high.
	{
		return (_workToDo <= 0) || _unsupportedMemory;
	}
	bool isAssigned() //If the task is assign or memory for performing it is very high.
	{
		return _isAssigned || _unsupportedMemory;
	}
	int getMemoryNeeded() //Accessor of required memory.
	{
		return _totalMemoryNeeded;
	}
private:
	int _workToDo; //Time period for task to end.
	bool _isAssigned; // Is the task is assigned to a thread?
	bool _unsupportedMemory; //If required memory for a thread to do this task is very high , then taskqueue will ignore it.

};

class MultiTask;

//Concrete Iterator for traversing tasks.Uses MultiTask as a collection.
class TaskIterator : public AbstractIterator
{
public:
	TaskIterator(const MultiTask *collection);
	void First();
	void Next();
	AbstractTask* CurrentItem() const;
	bool IsDone()const;
private:
	const MultiTask *_collection;
	int _current;

};





class MultiTask : public AbstractTask  //Composite Task class
{
public:

	TaskIterator* CreateIterator() { //Creates Task Iterator.
		return new TaskIterator(this);
	}

	MultiTask(string _NAME) //Constructor
	{
		_name = _NAME;
	}

	void add(AbstractTask* _TASK) //Add sub-task.
	{
		_innerTasks.push_back(_TASK);
	}

	AbstractTask* get(int _INDEX) const // Needed for iterator.
	{
		return _innerTasks[_INDEX];
	}

	int getCount() const //Needed for iterator.
	{
		return _innerTasks.size();
	}

	void Action(){} //Does nothing.
	
	void assignToPool() //Assign every sub-tasks to the thread pool.
	{
		TaskIterator* _iterator = this->CreateIterator();
		for(_iterator->First();!_iterator->IsDone();_iterator->Next())
		{
			if((*_iterator).CurrentItem() != NULL)
				(*_iterator).CurrentItem()->assignToPool();
		}
		delete _iterator;
	}
	
	bool isDone() //Is every sub-task is done?
	{
		bool _returner = true;
		TaskIterator* _iterator = this->CreateIterator();
		for (_iterator->First(); !_iterator->IsDone(); _iterator->Next())
		{
			if (!(*_iterator).CurrentItem()->isDone())
			{
				_returner = false;
				break;
			}
		}
		return _returner;
	}
	bool isAssigned() //Is every sub-task is assigned?
	{
		bool _returner = true;
		TaskIterator* _iterator = this->CreateIterator();
		for (_iterator->First(); !_iterator->IsDone(); _iterator->Next())
		{
			if (!(*_iterator).CurrentItem()->isAssigned())
			{
				_returner = false;
				break;
			}
		}
		return _returner;

	}

	int getMemoryNeeded() //Returns total memory of sub-tasks.
	{
		int totalMemory = 0;
		TaskIterator* _iterator = this->CreateIterator();
		for (_iterator->First(); !_iterator->IsDone(); _iterator->Next())
		{
			totalMemory += _iterator->CurrentItem()->getMemoryNeeded();
		}
		return totalMemory;

	}
private:
	vector<AbstractTask*> _innerTasks; //Container of sub-tasks.

};

class TaskQueue : public AbstractObserver //Container for waiting tasks and an observer of the threadpool.
{
public:
	void Update(int _INFO) //Observer method.
	{
		if(_INFO == 1 || _INFO == 2) //If a thread is created(1) or a thread is available(2).
			InsertToPool();

		//cout << "Task queue gets a notify code : " << _INFO	<< endl;
	}
	void InsertToPool() //Inserts front element to the thread pool.
						//If task is assigned it means it s going to be completed so pop it.
	{
		if(_innerTasks.empty())
			return;
		if(_innerTasks.front()->isAssigned())
			_innerTasks.pop();
		if(!_innerTasks.empty())
			{

				_innerTasks.front()->assignToPool();
			}
		

	}
	void Add(AbstractTask* _TASK) //Add task.
	{
		_innerTasks.push(_TASK);
	}

	bool isEmpty()
	{
		return _innerTasks.size() == 0;
	}

	int getCount()
	{
		return _innerTasks.size();
	}
private:
	queue<AbstractTask*> _innerTasks; //FIFO Task container.

};

//Memory manager which keeps track of total memory used by the thread pool.
class MemoryManager : public AbstractObserver
{

public:
	MemoryManager()
	{
		_totalMemory = 0;//Sum of memories.
	}
	void Update(int _INFO)
	{
		if(_INFO == 1 || _INFO == 3) // If a thread is created or existing thread is changed its value update current thread container.
		{	
			_observedThreads.clear();
			_totalMemory = 0;
			ThreadIterator* _iterator = ThreadPool::getInstance()->CreateIterator();
			for (_iterator->First(); !_iterator->IsDone(); _iterator->Next())
			{
				_observedThreads.push_back(*_iterator->CurrentItem());
				if(_iterator->CurrentItem()->getTask() !=  NULL)
					_totalMemory += _iterator->CurrentItem()->getTask()->getMemoryNeeded();

			}
			delete _iterator;

		}
		else if(_INFO == 4) //If runAll() method is executed then display totalMemory on console.
			displayTotalMemory();
	}

	int displayTotalMemory()
	{
		cout << "\t\t\t\t\t Total Memory : " << _totalMemory << "MB" << endl;
		if(_totalMemory >= 1000)
			writeLog();
	}

	void writeLog() // If total memory exceed 1GB of memory then write into log file with current tasks in it.
	{
		ofstream _file;
		_file.open("log.txt",ios_base::app);
		_file << "Total memory exceed 1 GB with value : " << _totalMemory << "MB" <<endl;
		_file << "Tasks assigned : " << endl;
		ThreadIterator* _iterator = ThreadPool::getInstance()->CreateIterator();
		for (_iterator->First(); !_iterator->IsDone(); _iterator->Next())
			{
				_observedThreads.push_back(*_iterator->CurrentItem());
				if(_iterator->CurrentItem()->getTask() !=  NULL)
					_file << _iterator->CurrentItem()->getTask()->getName() << "," ;
			}
		delete _iterator;


		_file.close();

	}



private:
	vector<Thread> _observedThreads; //Observers .
	int _totalMemory;	//Total memory of the thread pool.


};







//Bodies of some method in concrete iterator classes.
//---------------------------------------------
TaskIterator::TaskIterator(const MultiTask *collection) :
	_collection(collection), _current(0) {
}
void TaskIterator::First() {
	_current = 0;
}
void TaskIterator::Next() {
	_current++;
}
AbstractTask* TaskIterator::CurrentItem() const {
	return (IsDone() ? NULL : _collection->get(_current));
}
bool TaskIterator::IsDone() const {
	return _current >= _collection->getCount();
}

//-------------------------------------------
ThreadIterator::ThreadIterator(const ThreadPool *collection) :
	_collection(collection), _current(0) {
}

void ThreadIterator::First() {
	_current = 0;
}

void ThreadIterator::Next() {
	_current++;
}

bool ThreadIterator::IsDone() const {
	return _current >= _collection->getCount();
}

Thread* ThreadIterator::CurrentItem() const {
	return (IsDone() ? NULL : _collection->get(_current));
}


//-------------------------------------------

int main()
{
	//Creating sample Multitask.
	MultiTask* _task1 = new MultiTask("Shutting down");
	AbstractTask* _task2 = new Task("Saving open files",5,255);
	AbstractTask* _task3 = new Task("Shutting down open applications",8,241);
	AbstractTask* _task4 = new Task("Check OS updates",11,456);
	AbstractTask* _task5 = new Task("Shut down system",12,243);

	_task1->add(_task2);
	_task1->add(_task3);
	_task1->add(_task4);
	_task1->add(_task5);

	//Creating Independant Leaf Task samples.
	AbstractTask* _task6 = new Task("Deleting browser history",3,123);
	AbstractTask* _task7 = new Task("Handling calendar",2,56);
	AbstractTask* _task8 = new Task("Printing documents",4,77);

	TaskQueue* _tq1 = new TaskQueue(); //Creating new taskQueue
	MemoryManager* _mm1 = new MemoryManager(); //Creating new Memory Manager

	ThreadPool::getInstance()->Attach(_mm1); //Attaching memory manager to the thread pool as an observer
	ThreadPool::getInstance()->Attach(_tq1); // Attaching taskqueue to the thread pool as an observer
	_tq1->Add(_task1);
	
	_tq1->Add(_task6);
	_tq1->Add(_task7);
	_tq1->Add(_task8);
	
	ThreadPool::getInstance()->fillThePool(10,5); // filling the thread pool 
	
	

	while(true)
	{

		cout << "Type 1 to next period , 0 to exit ..  " << endl;
		int a ;
		cin >> a;
		
		if(a == 0)
			break;

		ThreadPool::getInstance()->runAll();
		cout <<"Working thread count : " << ThreadPool::getInstance()->getWorkingThreadCount() << endl;
		cout <<"-------------------------------------------------------------" << endl;

		if(ThreadPool::getInstance()->getWorkingThreadCount() <= 0)
		{
			cout << "All tasks are done.Existing.." << endl;
			break;
		}
	}

	
	return 0;
}
