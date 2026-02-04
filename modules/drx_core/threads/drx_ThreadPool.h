/*
  ==============================================================================

   This file is part of the DRX framework.
   Copyright (c) DinrusPro

   DRX is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the DRX framework, or combining the
   DRX framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the DRX End User Licence
   Agreement, and all incorporated terms including the DRX Privacy Policy and
   the DRX Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the DRX
   framework to you, and you must discontinue the installation or download
   process and cease use of the DRX framework.

   DRX End User Licence Agreement: https://drx.com/legal/drx-8-licence/
   DRX Privacy Policy: https://drx.com/drx-privacy-policy
   DRX Website Terms of Service: https://drx.com/drx-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace drx
{

class ThreadPool;

//==============================================================================
/**
    A task that is executed by a ThreadPool object.

    A ThreadPool keeps a list of ThreadPoolJob objects which are executed by
    its threads.

    The runJob() method needs to be implemented to do the task, and if the code that
    does the work takes a significant time to run, it must keep checking the shouldExit()
    method to see if something is trying to interrupt the job. If shouldExit() returns
    true, the runJob() method must return immediately.

    @see ThreadPool, Thread

    @tags{Core}
*/
class DRX_API  ThreadPoolJob
{
public:
    //==============================================================================
    /** Creates a thread pool job object.
        After creating your job, add it to a thread pool with ThreadPool::addJob().
    */
    explicit ThreadPoolJob (const Txt& name);

    /** Destructor. */
    virtual ~ThreadPoolJob();

    //==============================================================================
    /** Returns the name of this job.
        @see setJobName
    */
    Txt getJobName() const;

    /** Changes the job's name.
        @see getJobName
    */
    z0 setJobName (const Txt& newName);

    //==============================================================================
    /** These are the values that can be returned by the runJob() method.
    */
    enum JobStatus
    {
        jobHasFinished = 0,     /**< indicates that the job has finished and can be
                                     removed from the pool. */

        jobNeedsRunningAgain    /**< indicates that the job would like to be called
                                     again when a thread is free. */
    };

    /** Performs the actual work that this job needs to do.

        Your subclass must implement this method, in which is does its work.

        If the code in this method takes a significant time to run, it must repeatedly check
        the shouldExit() method to see if something is trying to interrupt the job.
        If shouldExit() ever returns true, the runJob() method must return immediately.

        If this method returns jobHasFinished, then the job will be removed from the pool
        immediately. If it returns jobNeedsRunningAgain, then the job will be left in the
        pool and will get a chance to run again as soon as a thread is free.

        @see shouldExit()
    */
    virtual JobStatus runJob() = 0;


    //==============================================================================
    /** Возвращает true, если this job is currently running its runJob() method. */
    b8 isRunning() const noexcept                     { return isActive; }

    /** Возвращает true, если something is trying to interrupt this job and make it stop.

        Your runJob() method must call this whenever it gets a chance, and if it ever
        returns true, the runJob() method must return immediately.

        @see signalJobShouldExit()
    */
    b8 shouldExit() const noexcept                    { return shouldStop; }

    /** Calling this will cause the shouldExit() method to return true, and the job
        should (if it's been implemented correctly) stop as soon as possible.

        @see shouldExit()
    */
    z0 signalJobShouldExit();

    /** Add a listener to this thread job which will receive a callback when
        signalJobShouldExit was called on this thread job.

        @see signalJobShouldExit, removeListener
    */
    z0 addListener (Thread::Listener*);

    /** Removes a listener added with addListener. */
    z0 removeListener (Thread::Listener*);

    //==============================================================================
    /** If the calling thread is being invoked inside a runJob() method, this will
        return the ThreadPoolJob that it belongs to.
    */
    static ThreadPoolJob* getCurrentThreadPoolJob();

    //==============================================================================
private:
    friend class ThreadPool;
    Txt jobName;
    ThreadPool* pool = nullptr;
    std::atomic<b8> shouldStop { false }, isActive { false }, shouldBeDeleted { false };
    ThreadSafeListenerList<Thread::Listener> listeners;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreadPoolJob)
};

//==============================================================================
/**
    A set of threads that will run a list of jobs.

    When a ThreadPoolJob object is added to the ThreadPool's list, its runJob() method
    will be called by the next pooled thread that becomes free.

    @see ThreadPoolJob, Thread

    @tags{Core}
*/
struct ThreadPoolOptions
{
    /** The name to give each thread in the pool. */
    [[nodiscard]] ThreadPoolOptions withThreadName (Txt newThreadName) const
    {
        return withMember (*this, &ThreadPoolOptions::threadName, newThreadName);
    }

    /** The number of threads to run.
        These will be started when a pool is created, and run until the pool is destroyed.
    */
    [[nodiscard]] ThreadPoolOptions withNumberOfThreads (i32 newNumberOfThreads) const
    {
        return withMember (*this, &ThreadPoolOptions::numberOfThreads, newNumberOfThreads);
    }

    /** The size of the stack of each thread in the pool. */
    [[nodiscard]] ThreadPoolOptions withThreadStackSizeBytes (size_t newThreadStackSizeBytes) const
    {
        return withMember (*this, &ThreadPoolOptions::threadStackSizeBytes, newThreadStackSizeBytes);
    }

    /** The desired priority of each thread in the pool. */
    [[nodiscard]] ThreadPoolOptions withDesiredThreadPriority (Thread::Priority newDesiredThreadPriority) const
    {
        return withMember (*this, &ThreadPoolOptions::desiredThreadPriority, newDesiredThreadPriority);
    }

    Txt threadName { "Pool" };
    i32 numberOfThreads { SystemStats::getNumCpus() };
    size_t threadStackSizeBytes { Thread::osDefaultStackSize };
    Thread::Priority desiredThreadPriority { Thread::Priority::normal };
};


//==============================================================================
/**
    A set of threads that will run a list of jobs.

    When a ThreadPoolJob object is added to the ThreadPool's list, its runJob() method
    will be called by the next pooled thread that becomes free.

    @see ThreadPoolJob, Thread

    @tags{Core}
*/
class DRX_API  ThreadPool
{
public:
    using Options = ThreadPoolOptions;

    //==============================================================================
    /** Creates a thread pool based on the provided options.
        Once you've created a pool, you can give it some jobs by calling addJob().

        @see ThreadPool::ThreadPoolOptions
    */
    explicit ThreadPool (const Options& options);

    /** Creates a thread pool based using the default arguments provided by
        ThreadPoolOptions.

        Once you've created a pool, you can give it some jobs by calling addJob().

        @see ThreadPoolOptions
    */
    ThreadPool() : ThreadPool { Options{} } {}

    /** Creates a thread pool.
        Once you've created a pool, you can give it some jobs by calling addJob().

        @param numberOfThreads       the number of threads to run. These will be started
                                     immediately, and will run until the pool is deleted.
        @param threadStackSizeBytes  the size of the stack of each thread. If this value
                                     is zero then the default stack size of the OS will
                                     be used.
        @param desiredThreadPriority the desired priority of each thread in the pool.
    */
    ThreadPool (i32 numberOfThreads,
                size_t threadStackSizeBytes = Thread::osDefaultStackSize,
                Thread::Priority desiredThreadPriority = Thread::Priority::normal);

    /** Destructor.

        This will attempt to remove all the jobs before deleting, but if you want to
        specify a timeout, you should call removeAllJobs() explicitly before deleting
        the pool.
    */
    ~ThreadPool();

    //==============================================================================
    /** A callback class used when you need to select which ThreadPoolJob objects are suitable
        for some kind of operation.
        @see ThreadPool::removeAllJobs
    */
    class DRX_API  JobSelector
    {
    public:
        virtual ~JobSelector() = default;

        /** Should return true if the specified thread matches your criteria for whatever
            operation that this object is being used for.

            Any implementation of this method must be extremely fast and thread-safe!
        */
        virtual b8 isJobSuitable (ThreadPoolJob* job) = 0;
    };

    //==============================================================================
    /** Adds a job to the queue.

        Once a job has been added, then the next time a thread is free, it will run
        the job's ThreadPoolJob::runJob() method. Depending on the return value of the
        runJob() method, the pool will either remove the job from the pool or add it to
        the back of the queue to be run again.

        If deleteJobWhenFinished is true, then the job object will be owned and deleted by
        the pool when not needed - if you do this, make sure that your object's destructor
        is thread-safe.

        If deleteJobWhenFinished is false, the pointer will be used but not deleted, and
        the caller is responsible for making sure the object is not deleted before it has
        been removed from the pool.
    */
    z0 addJob (ThreadPoolJob* job,
                 b8 deleteJobWhenFinished);

    /** Adds a lambda function to be called as a job.
        This will create an internal ThreadPoolJob object to encapsulate and call the lambda.
    */
    z0 addJob (std::function<ThreadPoolJob::JobStatus()> job);

    /** Adds a lambda function to be called as a job.
        This will create an internal ThreadPoolJob object to encapsulate and call the lambda.
    */
    z0 addJob (std::function<z0()> job);

    /** Tries to remove a job from the pool.

        If the job isn't yet running, this will simply remove it. If it is running, it
        will wait for it to finish.

        If the timeout period expires before the job finishes running, then the job will be
        left in the pool and this will return false. It returns true if the job is successfully
        stopped and removed.

        @param job                  the job to remove
        @param interruptIfRunning   if true, then if the job is currently busy, its
                                    ThreadPoolJob::signalJobShouldExit() method will be called to try
                                    to interrupt it. If false, then if the job will be allowed to run
                                    until it stops normally (or the timeout expires)
        @param timeOutMilliseconds  the length of time this method should wait for the job to finish
                                    before giving up and returning false
    */
    b8 removeJob (ThreadPoolJob* job,
                    b8 interruptIfRunning,
                    i32 timeOutMilliseconds);

    /** Tries to remove all jobs from the pool.

        @param interruptRunningJobs if true, then all running jobs will have their ThreadPoolJob::signalJobShouldExit()
                                    methods called to try to interrupt them
        @param timeOutMilliseconds  the length of time this method should wait for all the jobs to finish
                                    before giving up and returning false
        @param selectedJobsToRemove if this is not a nullptr, the JobSelector object is asked to decide
                                    which jobs should be removed. If it is a nullptr, all jobs are removed
        @returns    true if all jobs are successfully stopped and removed; false if the timeout period
                    expires while waiting for one or more jobs to stop
    */
    b8 removeAllJobs (b8 interruptRunningJobs,
                        i32 timeOutMilliseconds,
                        JobSelector* selectedJobsToRemove = nullptr);

    /** Returns the number of jobs currently running or queued. */
    i32 getNumJobs() const noexcept;

    /** Returns the number of threads assigned to this thread pool. */
    i32 getNumThreads() const noexcept;

    /** Returns one of the jobs in the queue.

        Note that this can be a very volatile list as jobs might be continuously getting shifted
        around in the list, and this method may return nullptr if the index is currently out-of-range.
    */
    ThreadPoolJob* getJob (i32 index) const noexcept;

    /** Возвращает true, если the given job is currently queued or running.

        @see isJobRunning()
    */
    b8 contains (const ThreadPoolJob* job) const noexcept;

    /** Возвращает true, если the given job is currently being run by a thread. */
    b8 isJobRunning (const ThreadPoolJob* job) const noexcept;

    /** Waits until a job has finished running and has been removed from the pool.

        This will wait until the job is no longer in the pool - i.e. until its
        runJob() method returns ThreadPoolJob::jobHasFinished.

        If the timeout period expires before the job finishes, this will return false;
        it returns true if the job has finished successfully.
    */
    b8 waitForJobToFinish (const ThreadPoolJob* job,
                             i32 timeOutMilliseconds) const;

    /** If the given job is in the queue, this will move it to the front so that it
        is the next one to be executed.
    */
    z0 moveJobToFront (const ThreadPoolJob* jobToMove) noexcept;

    /** Returns a list of the names of all the jobs currently running or queued.
        If onlyReturnActiveJobs is true, only the ones currently running are returned.
    */
    StringArray getNamesOfAllJobs (b8 onlyReturnActiveJobs) const;

private:
    //==============================================================================
    Array<ThreadPoolJob*> jobs;

    struct ThreadPoolThread;
    friend class ThreadPoolJob;
    OwnedArray<ThreadPoolThread> threads;

    CriticalSection lock;
    WaitableEvent jobFinishedSignal;

    b8 runNextJob (ThreadPoolThread&);
    ThreadPoolJob* pickNextJobToRun();
    z0 addToDeleteList (OwnedArray<ThreadPoolJob>&, ThreadPoolJob*) const;
    z0 stopThreads();

    // Note that this method has changed, and no longer has a parameter to indicate
    // whether the jobs should be deleted - see the new method for details.
    z0 removeAllJobs (b8, i32, b8);

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreadPool)
};

} // namespace drx
