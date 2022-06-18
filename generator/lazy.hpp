#include <functional>
#include <optional>

/**
 * @brief Lazy value that is computed using a retriever function the first time
 * it is accessed.
 */
template <typename T>
class lazy
{
private:
    std::optional<std::function<T()>> retriever_;
    mutable std::optional<T> value_;

public:
    lazy() noexcept = default;
    lazy(std::function<T()> retriever) noexcept : retriever_{retriever} {}

    /**
     * @brief Get the value of the lazy object.
     *
     * If the value is accessed for the first time, it will be computed first
     * using the retriever.
     *
     * @exception lazy::no_retriever No retriever was set for this lazy object.
     */
    operator const T &() const
    {
        if (value_.has_value())
        {
            return value_.value();
        }
        else if (retriever_.has_value())
        {
            value_ = retriever_.value()();
            return value_.value();
        }
        else
        {
            throw no_retriever();
        }
    }

    /**
     * @brief Get the value of the lazy object.
     *
     * If the value is accessed for the first time, it will be computed first
     * using the retriever.
     *
     * @param retriever The retriever to use when computing the value.
     */
    const T &value_or(std::function<T()> retriever) const noexcept
    {
        if (value_.has_value())
        {
            return value_.value();
        }
        else
        {
            value_ = retriever();
            return value_.value();
        }
    }

    class no_retriever : public std::exception
    {
    public:
        const char *what() const noexcept override
        {
            return "No retriever was set for this lazy value.";
        }
    };
};