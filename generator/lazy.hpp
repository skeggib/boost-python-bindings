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
    std::function<T()> retriever_;
    mutable std::optional<T> value_;

public:
    lazy(std::function<T()> retriever) noexcept : retriever_{retriever} {}

    operator const T &() const
    {
        return value();
    }

    /**
     * @brief Get the value of the lazy object.
     *
     * If the value is accessed for the first time, it will be computed first
     * using the retriever.
     */
    const T &value() const noexcept
    {
        if (!value_.has_value())
        {
            value_ = retriever_();
        }
        return value_.value();
    }
};

/**
 * @brief Lazy value placeholder that does not has a retriever yet.
 * 
 * It will be converted to a lazy value when set_retriever is called. The
 * created lazy value will be stored in the lazy placeholder for future use.
 */
template <typename T>
class lazy_placeholder
{
private:
    mutable std::optional<lazy<T>> lazy_;
public:
    lazy<T> set_retriever(std::function<T()> retriever) const
    {
        if (!lazy_.has_value())
        {
            lazy_ = lazy<T>(retriever);
        }
        return lazy_.value();
    }
};
