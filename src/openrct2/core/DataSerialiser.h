/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include "DataSerialiserTraits.h"
#include "MemoryStream.h"

#include <iostream>
#include <type_traits>
#include <variant>

class BinarySerialiser
{
private:
    std::variant<std::istream*, std::ostream*> _stream{};

public:
    BinarySerialiser(std::istream* stream)
        : _stream(stream)
    {
    }

    BinarySerialiser(std::ostream* stream)
        : _stream(stream)
    {
    }

    bool IsReading() const
    {
        return std::holds_alternative<std::istream*>(_stream);
    }

    bool IsWriting() const
    {
        return std::holds_alternative<std::ostream*>(_stream);
    }

    template<typename T> BinarySerialiser& operator<<(const T& data)
    {
        if (auto istream = std::get_if<std::istream*>(&_stream))
        {
            DataSerializerTraits<T>::decode(**istream, const_cast<T&>(data));
        }
        else if (auto ostream = std::get_if<std::ostream*>(&_stream))
        {
            DataSerializerTraits<T>::encode(**ostream, data);
        }
        return *this;
    }
};

class BinaryReader : public BinarySerialiser
{
public:
    BinaryReader(std::istream* stream)
        : BinarySerialiser(stream)
    {
    }
};

class BinaryWriter : public BinarySerialiser
{
public:
    BinaryWriter(std::ostream* stream)
        : BinarySerialiser(stream)
    {
    }
};

class DataSerialiser
{
private:
    OpenRCT2::MemoryStream _stream;
    OpenRCT2::IStream& _activeStream;
    bool _isSaving = false;
    bool _isLogging = false;

public:
    DataSerialiser(bool isSaving)
        : _activeStream(_stream)
        , _isSaving(isSaving)
        , _isLogging(false)
    {
    }

    DataSerialiser(bool isSaving, OpenRCT2::IStream& stream, bool isLogging = false)
        : _activeStream(stream)
        , _isSaving(isSaving)
        , _isLogging(isLogging)
    {
    }

    bool IsSaving() const
    {
        return _isSaving;
    }

    bool IsLoading() const
    {
        return !_isSaving;
    }

    bool IsLogging() const
    {
        return _isLogging;
    }

    OpenRCT2::IStream& GetStream()
    {
        return _activeStream;
    }

    template<typename T> DataSerialiser& operator<<(const T& data)
    {
        if (!_isLogging)
        {
            if (_isSaving)
                DataSerializerTraits<T>::encode(&_activeStream, data);
            else
                DataSerializerTraits<T>::decode(&_activeStream, const_cast<T&>(data));
        }
        else
        {
            DataSerializerTraits<T>::log(&_activeStream, data);
        }

        return *this;
    }

    template<typename T> DataSerialiser& operator<<(DataSerialiserTag<T> data)
    {
        if (!_isLogging)
        {
            if (_isSaving)
                DataSerializerTraits<DataSerialiserTag<T>>::encode(&_activeStream, data);
            else
                DataSerializerTraits<DataSerialiserTag<T>>::decode(&_activeStream, data);
        }
        else
        {
            DataSerializerTraits<DataSerialiserTag<T>>::log(&_activeStream, data);
        }

        return *this;
    }
};
