#ifdef ME_METAL

#include "depth_stencil_state_metal.h"

ME::DepthStencilStateMetal::DepthStencilStateMetal() {}

ME::DepthStencilStateMetal::~DepthStencilStateMetal() {}

MTL::DepthStencilState* ME::DepthStencilStateMetal::GetNewDepthStencilState(
    MTL::Device* device, ME::DepthCompareFunction depthCompareFunction, bool depthWriteEnabled) {
    MTL::DepthStencilDescriptor* dsDesc = MTL::DepthStencilDescriptor::alloc()->init();

    switch (depthCompareFunction) {
        case ME::DepthCompareFunction::Never:
            dsDesc->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionNever);
            break;
        case ME::DepthCompareFunction::Less:
            dsDesc->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionLess);
            break;
        case ME::DepthCompareFunction::Equal:
            dsDesc->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionEqual);
            break;
        case ME::DepthCompareFunction::LessEqual:
            dsDesc->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionLessEqual);
            break;
        case ME::DepthCompareFunction::Greater:
            dsDesc->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionGreater);
            break;
        case ME::DepthCompareFunction::NotEqual:
            dsDesc->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionNotEqual);
            break;
        case ME::DepthCompareFunction::GreaterEqual:
            dsDesc->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionGreaterEqual);
            break;
        case ME::DepthCompareFunction::Always:
            dsDesc->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionAlways);
            break;
        default:
            dsDesc->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionLess);
            break;
    }

    dsDesc->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionLess);

    dsDesc->setDepthWriteEnabled(depthWriteEnabled);
    MTL::DepthStencilState* dss = device->newDepthStencilState(dsDesc);
    dsDesc->release();
    return dss;
}

MTL::DepthStencilState* ME::DepthStencilStateMetal::GetNewDepthStencilState2D(MTL::Device* device) {
    return GetNewDepthStencilState(device, ME::DepthCompareFunction::Always, false);
}

#endif
