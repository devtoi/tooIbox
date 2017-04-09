#include "Job.h"

tooibox::Job::Job(tooibox::Job::Function jobFunction, tooibox::Job *parent)
    : m_function{jobFunction}, m_parent{parent}, m_unfinishedJobs{1}
{
    // Make sure parent is able to wait for us to finish
    if (parent != nullptr)
        parent->m_unfinishedJobs++;
}

void tooibox::Job::Run()
{
    m_function(*this);
    Finish();
}

bool tooibox::Job::IsFinished() const
{
    return m_unfinishedJobs == 0;
}

void tooibox::Job::Finish()
{
    m_unfinishedJobs--;
    // Tell parent we are finished
    if (IsFinished() && m_parent != nullptr)
        m_parent->Finish();
}
