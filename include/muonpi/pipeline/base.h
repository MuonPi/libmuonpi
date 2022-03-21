#ifndef MUONPI_PIPELINE_BASE_H
#define MUONPI_PIPELINE_BASE_H

#include "muonpi/global.h"
#include "muonpi/sink/base.h"
#include "muonpi/source/base.h"

namespace muonpi::pipeline {

template <typename T>
class LIBMUONPI_PUBLIC base
    : public sink::base<T>,
      public source::base<T> {
public:
    explicit base(sink::base<T>& sink);
};

template <typename T>
base<T>::base(sink::base<T>& sink)
    : source::base<T>(sink)
{
}

} // namespace muonpi::pipeline

#endif // MUONPI_PIPELINE_BASE_H
