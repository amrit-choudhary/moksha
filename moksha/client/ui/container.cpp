#include "container.h"

ME::Container::Container() {}

ME::Container::~Container() {}

ME::UIElementType ME::Container::GetType() const {
    return UIElementType::Container;
}
