#pragma once

#include <exception>
#include <cstring>

class DogDealerException : public std::exception
{
public:
    DogDealerException() = default;
    DogDealerException(char const * user_message, bool copy_string = true)
    {
        if( copy_string )
        {
            auto size = strlen(user_message);
            auto initial_message = new char[size + 1];
            memcpy(initial_message, user_message, size + 1);
            this->message = initial_message;
            this->must_free = true;
        }
        else
        {
            this->message = user_message;
            this->must_free = false;
        }
    }
    DogDealerException(DogDealerException const & other) :
        DogDealerException(other.message, other.must_free)
    {
    }
    DogDealerException(DogDealerException && other) :
        message(other.message),
        must_free(other.must_free)
    {
		other.message = nullptr;
		other.must_free = false;
    }
    ~DogDealerException()
    {
        if(must_free) delete message;
    }
    char const * what() const final
    {
        return message;
    }

private:
    // null terminated message
    char const * message = nullptr;
    bool must_free = false;
};