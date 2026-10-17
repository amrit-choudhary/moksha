#pragma once

/**
 * Container UI widget: a layout-only grouping node with no owned visual. Gives that role
 * (already possible via UIElement's own defaults) a clear name at the call site.
 */

#include "ui_element.h"

namespace ME {

class Container : public UIElement {
   public:
    Container();
    ~Container();

    UIElementType GetType() const override;
};

}  // namespace ME
