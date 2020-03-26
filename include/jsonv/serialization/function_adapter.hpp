/// \file jsonv/serialization/function_adapter.hpp
///
/// Copyright (c) 2015-2020 by Travis Gockel. All rights reserved.
///
/// This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
/// as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
/// version.
///
/// \author Travis Gockel (travis@gockelhut.com)
#pragma once

#include <jsonv/config.hpp>
#include <jsonv/serialization.hpp>

#include "adapter_for.hpp"

namespace jsonv
{

/// \addtogroup Serialization
/// \{

template <typename T, typename FExtract, typename FToJson>
class function_adapter :
        public adapter_for<T>
{
public:
    template <typename FUExtract, typename FUToJson>
    explicit function_adapter(FUExtract&& extract_, FUToJson&& to_json_) :
            _extract(std::forward<FUExtract>(extract_)),
            _to_json(std::forward<FUToJson>(to_json_))
    { }

protected:
virtual result<T, void> create(extraction_context& context, reader& from) const override
    {
        using create_result_type = decltype(create_impl(_extract, context, from));

        if constexpr (is_result_v<create_result_type>)
        {
            return create_impl(_extract, context, from);
        }
        else
        {
            return ok{ create_impl(_extract, context, from) };
        }
    }

    virtual value to_json(const serialization_context& context, const T& from) const override
    {
        return to_json_impl(_to_json, context, from);
    }

private:
    template <typename FUExtract>
    static auto create_impl(const FUExtract& func, extraction_context& context, reader& from)
            -> decltype(func(context, from))
    {
        return func(context, from);
    }

    template <typename FUExtract, typename = void>
    static auto create_impl(const FUExtract& func, extraction_context&, reader& from)
            -> decltype(func(from))
    {
        return func(from);
    }

    template <typename FUToJson>
    static auto to_json_impl(const FUToJson& func, const serialization_context& context, const T& from)
            -> decltype(func(context, from))
    {
        return func(context, from);
    }

    template <typename FUToJson, typename = void>
    static auto to_json_impl(const FUToJson& func, const serialization_context&, const T& from)
            -> decltype(func(from))
    {
        return func(from);
    }

private:
    FExtract _extract;
    FToJson  _to_json;
};

// TODO(#150): These should be deduction guides _and_ have some checking for `result`
template <typename FExtract, typename FToJson>
auto make_adapter(FExtract extract, FToJson to_json_)
    -> function_adapter<decltype(extract(std::declval<extraction_context&>(), std::declval<reader&>())),
                        FExtract,
                        FToJson
                       >
{
    return function_adapter<decltype(extract(std::declval<extraction_context&>(), std::declval<reader&>())),
                            FExtract,
                            FToJson
                           >
            (std::move(extract), std::move(to_json_));
}

template <typename FExtract, typename FToJson, typename = void>
auto make_adapter(FExtract extract, FToJson to_json_)
    -> function_adapter<decltype(extract(std::declval<reader&>())),
                        FExtract,
                        FToJson
                       >
{
    return function_adapter<decltype(extract(std::declval<reader&>())),
                            FExtract,
                            FToJson
                           >
            (std::move(extract), std::move(to_json_));
}

/// \}

}
