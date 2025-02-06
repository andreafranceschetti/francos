#include <memory>

namespace francos {


template <typename T>
class Publisher {
    using SharedPtr = std::shared_ptr<Publisher<T>>;

};

}