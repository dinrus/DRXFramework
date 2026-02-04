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

struct ThreadPool::ThreadPoolThread final : public Thread
{
    ThreadPoolThread (ThreadPool& p, const Options& options)
       : Thread { options.threadName, options.threadStackSizeBytes },
         pool { p }
    {
    }

    z0 run() override
    {
        while (! threadShouldExit())
        {
            if (! pool.runNextJob (*this))
                wait (500);
        }
    }

    std::atomic<ThreadPoolJob*> currentJob { nullptr };

    ThreadPool& pool;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreadPoolThread)
};

//==============================================================================
ThreadPoolJob::ThreadPoolJob (const Txt& name)  : jobName (name)
{
}

ThreadPoolJob::~ThreadPoolJob()
{
    // you mustn't delete a job while it's still in a pool! Use ThreadPool::removeJob()
    // to remove it first!
    jassert (pool == nullptr || ! pool->contains (this));
}

Txt ThreadPoolJob::getJobName() const
{
    return jobName;
}

z0 ThreadPoolJob::setJobName (const Txt& newName)
{
    jobName = newName;
}

z0 ThreadPoolJob::signalJobShouldExit()
{
    shouldStop = true;
    listeners.call ([] (Thread::Listener& l) { l.exitSignalSent(); });
}

z0 ThreadPoolJob::addListener (Thread::Listener* listener)
{
    listeners.add (listener);
}

z0 ThreadPoolJob::removeListener (Thread::Listener* listener)
{
    listeners.remove (listener);
}

ThreadPoolJob* ThreadPoolJob::getCurrentThreadPoolJob()
{
    if (auto* t = dynamic_cast<ThreadPool::ThreadPoolThread*> (Thread::getCurrentThread()))
        return t->currentJob.load();

    return nullptr;
}

//==============================================================================
ThreadPool::ThreadPool (const Options& options)
{
    // not much point having a pool without any threads!
    jassert (options.numberOfThreads > 0);

    for (i32 i = jmax (1, options.numberOfThreads); --i >= 0;)
        threads.add (new ThreadPoolThread (*this, options));

    for (auto* t : threads)
        t->startThread (options.desiredThreadPriority);
}

ThreadPool::ThreadPool (i32 numberOfThreads,
                        size_t threadStackSizeBytes,
                        Thread::Priority desiredThreadPriority)
    : ThreadPool { Options{}.withNumberOfThreads (numberOfThreads)
                            .withThreadStackSizeBytes (threadStackSizeBytes)
                            .withDesiredThreadPriority (desiredThreadPriority) }
{
}

ThreadPool::~ThreadPool()
{
    removeAllJobs (true, 5000);
    stopThreads();
}

z0 ThreadPool::stopThreads()
{
    for (auto* t : threads)
        t->signalThreadShouldExit();

    for (auto* t : threads)
        t->stopThread (500);
}

z0 ThreadPool::addJob (ThreadPoolJob* job, b8 deleteJobWhenFinished)
{
    jassert (job != nullptr);
    jassert (job->pool == nullptr);

    if (job->pool == nullptr)
    {
        job->pool = this;
        job->shouldStop = false;
        job->isActive = false;
        job->shouldBeDeleted = deleteJobWhenFinished;

        {
            const ScopedLock sl (lock);
            jobs.add (job);
        }

        for (auto* t : threads)
            t->notify();
    }
}

z0 ThreadPool::addJob (std::function<ThreadPoolJob::JobStatus()> jobToRun)
{
    struct LambdaJobWrapper final : public ThreadPoolJob
    {
        LambdaJobWrapper (std::function<ThreadPoolJob::JobStatus()> j) : ThreadPoolJob ("lambda"), job (j) {}
        JobStatus runJob() override      { return job(); }

        std::function<ThreadPoolJob::JobStatus()> job;
    };

    addJob (new LambdaJobWrapper (jobToRun), true);
}

z0 ThreadPool::addJob (std::function<z0()> jobToRun)
{
    struct LambdaJobWrapper final : public ThreadPoolJob
    {
        LambdaJobWrapper (std::function<z0()> j) : ThreadPoolJob ("lambda"), job (std::move (j)) {}
        JobStatus runJob() override      { job(); return ThreadPoolJob::jobHasFinished; }

        std::function<z0()> job;
    };

    addJob (new LambdaJobWrapper (std::move (jobToRun)), true);
}

i32 ThreadPool::getNumJobs() const noexcept
{
    const ScopedLock sl (lock);
    return jobs.size();
}

i32 ThreadPool::getNumThreads() const noexcept
{
    return threads.size();
}

ThreadPoolJob* ThreadPool::getJob (i32 index) const noexcept
{
    const ScopedLock sl (lock);
    return jobs [index];
}

b8 ThreadPool::contains (const ThreadPoolJob* job) const noexcept
{
    const ScopedLock sl (lock);
    return jobs.contains (const_cast<ThreadPoolJob*> (job));
}

b8 ThreadPool::isJobRunning (const ThreadPoolJob* job) const noexcept
{
    const ScopedLock sl (lock);
    return jobs.contains (const_cast<ThreadPoolJob*> (job)) && job->isActive;
}

z0 ThreadPool::moveJobToFront (const ThreadPoolJob* job) noexcept
{
    const ScopedLock sl (lock);

    auto index = jobs.indexOf (const_cast<ThreadPoolJob*> (job));

    if (index > 0 && ! job->isActive)
        jobs.move (index, 0);
}

b8 ThreadPool::waitForJobToFinish (const ThreadPoolJob* job, i32 timeOutMs) const
{
    if (job != nullptr)
    {
        auto start = Time::getMillisecondCounter();

        while (contains (job))
        {
            if (timeOutMs >= 0 && Time::getMillisecondCounter() >= start + (u32) timeOutMs)
                return false;

            jobFinishedSignal.wait (2);
        }
    }

    return true;
}

b8 ThreadPool::removeJob (ThreadPoolJob* job, b8 interruptIfRunning, i32 timeOutMs)
{
    b8 dontWait = true;
    OwnedArray<ThreadPoolJob> deletionList;

    if (job != nullptr)
    {
        const ScopedLock sl (lock);

        if (jobs.contains (job))
        {
            if (job->isActive)
            {
                if (interruptIfRunning)
                    job->signalJobShouldExit();

                dontWait = false;
            }
            else
            {
                jobs.removeFirstMatchingValue (job);
                addToDeleteList (deletionList, job);
            }
        }
    }

    return dontWait || waitForJobToFinish (job, timeOutMs);
}

b8 ThreadPool::removeAllJobs (b8 interruptRunningJobs, i32 timeOutMs,
                                ThreadPool::JobSelector* selectedJobsToRemove)
{
    Array<ThreadPoolJob*> jobsToWaitFor;

    {
        OwnedArray<ThreadPoolJob> deletionList;

        {
            const ScopedLock sl (lock);

            for (i32 i = jobs.size(); --i >= 0;)
            {
                auto* job = jobs.getUnchecked (i);

                if (selectedJobsToRemove == nullptr || selectedJobsToRemove->isJobSuitable (job))
                {
                    if (job->isActive)
                    {
                        jobsToWaitFor.add (job);

                        if (interruptRunningJobs)
                            job->signalJobShouldExit();
                    }
                    else
                    {
                        jobs.remove (i);
                        addToDeleteList (deletionList, job);
                    }
                }
            }
        }
    }

    auto start = Time::getMillisecondCounter();

    for (;;)
    {
        for (i32 i = jobsToWaitFor.size(); --i >= 0;)
        {
            auto* job = jobsToWaitFor.getUnchecked (i);

            if (! isJobRunning (job))
                jobsToWaitFor.remove (i);
        }

        if (jobsToWaitFor.size() == 0)
            break;

        if (timeOutMs >= 0 && Time::getMillisecondCounter() >= start + (u32) timeOutMs)
            return false;

        jobFinishedSignal.wait (20);
    }

    return true;
}

StringArray ThreadPool::getNamesOfAllJobs (b8 onlyReturnActiveJobs) const
{
    StringArray s;
    const ScopedLock sl (lock);

    for (auto* job : jobs)
        if (job->isActive || ! onlyReturnActiveJobs)
            s.add (job->getJobName());

    return s;
}

ThreadPoolJob* ThreadPool::pickNextJobToRun()
{
    OwnedArray<ThreadPoolJob> deletionList;

    {
        const ScopedLock sl (lock);

        for (i32 i = 0; i < jobs.size(); ++i)
        {
            if (auto* job = jobs[i])
            {
                if (! job->isActive)
                {
                    if (job->shouldStop)
                    {
                        jobs.remove (i);
                        addToDeleteList (deletionList, job);
                        --i;
                        continue;
                    }

                    job->isActive = true;
                    return job;
                }
            }
        }
    }

    return nullptr;
}

b8 ThreadPool::runNextJob (ThreadPoolThread& thread)
{
    if (auto* job = pickNextJobToRun())
    {
        auto result = ThreadPoolJob::jobHasFinished;
        thread.currentJob = job;

        try
        {
            result = job->runJob();
        }
        catch (...)
        {
            jassertfalse; // Your runJob() method mustn't throw any exceptions!
        }

        thread.currentJob = nullptr;

        OwnedArray<ThreadPoolJob> deletionList;

        {
            const ScopedLock sl (lock);

            if (jobs.contains (job))
            {
                job->isActive = false;

                if (result != ThreadPoolJob::jobNeedsRunningAgain || job->shouldStop)
                {
                    jobs.removeFirstMatchingValue (job);
                    addToDeleteList (deletionList, job);

                    jobFinishedSignal.signal();
                }
                else
                {
                    // move the job to the end of the queue if it wants another go
                    jobs.move (jobs.indexOf (job), -1);
                }
            }
        }

        return true;
    }

    return false;
}

z0 ThreadPool::addToDeleteList (OwnedArray<ThreadPoolJob>& deletionList, ThreadPoolJob* job) const
{
    job->shouldStop = true;
    job->pool = nullptr;

    if (job->shouldBeDeleted)
        deletionList.add (job);
}

} // namespace drx
