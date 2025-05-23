// Copyright 2009-present MongoDB, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <cstdlib>
#include <memory>
#include <type_traits>

#include <bsoncxx/document/value-fwd.hpp>

#include <bsoncxx/array/view.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/stdx/type_traits.hpp>

#include <bsoncxx/config/prelude.hpp>

namespace bsoncxx {
namespace v_noabi {
namespace document {

///
/// A read-only BSON document that owns its underlying buffer.
///
/// When a document::value goes out of scope, the underlying buffer is freed. Generally this class
/// should be used sparingly; document::view should be used instead wherever possible.
///
class value {
   public:
    using deleter_type = void(BSONCXX_ABI_CDECL*)(std::uint8_t*);
    using unique_ptr_type = std::unique_ptr<uint8_t[], deleter_type>;

    ///
    /// Constructs a value from a buffer.
    /// This constructor transfers ownership of the buffer to the resulting
    /// value. A user-provided deleter is used to destroy the buffer.
    ///
    /// @param data
    ///   A pointer to a buffer containing a valid BSON document.
    /// @param length
    ///   The length of the document.
    /// @param dtor
    ///   A user provided deleter.
    ///
    BSONCXX_ABI_EXPORT_CDECL() value(std::uint8_t* data, std::size_t length, deleter_type dtor);

    ///
    /// Constructs a value from a std::unique_ptr to a buffer. The ownership
    /// of the buffer is transferred to the constructed value.
    ///
    /// @param ptr
    ///   A pointer to a buffer containing a valid BSON document.
    /// @param length
    ///   The length of the document.
    ///
    BSONCXX_ABI_EXPORT_CDECL() value(unique_ptr_type ptr, std::size_t length);

    ///
    /// Constructs a value from a view of a document. The data referenced
    /// by the document::view will be copied into a new buffer managed by the
    /// constructed value.
    ///
    /// @param view
    ///   A view of another document to copy.
    ///
    explicit BSONCXX_ABI_EXPORT_CDECL() value(document::view view);

    ~value() = default;

    BSONCXX_ABI_EXPORT_CDECL() value(value const&);
    BSONCXX_ABI_EXPORT_CDECL(value&) operator=(value const&);

    value(value&&) = default;
    value& operator=(value&&) = default;

    ///
    /// Constructor used for serialization of user objects. This uses argument-dependent lookup
    /// to find the function declaration
    /// `void to_bson(T& t, bsoncxx::v_noabi::document::value doc)`.
    ///
    /// @param t
    ///   A user-defined object to serialize into a BSON object.
    ///
    template <typename T, detail::requires_not_t<int, std::is_same<T, array::view>> = 0>
    explicit value(T const& t) : value({}) {
        to_bson(t, *this);
    }
    template <typename T>
    value& operator=(T const& t) {
        *this = value{t};
        return *this;
    }

    ///
    /// @returns A const_iterator to the first element of the document.
    ///
    BSONCXX_ABI_EXPORT_CDECL(document::view::const_iterator) cbegin() const;

    ///
    /// @returns A const_iterator to the past-the-end element of the document.
    ///
    BSONCXX_ABI_EXPORT_CDECL(document::view::const_iterator) cend() const;

    ///
    /// @returns A const_iterator to the first element of the document.
    ///
    BSONCXX_ABI_EXPORT_CDECL(document::view::const_iterator) begin() const;

    ///
    /// @returns A const_iterator to the past-the-end element of the document.
    ///
    BSONCXX_ABI_EXPORT_CDECL(document::view::const_iterator) end() const;

    ///
    /// Finds the first element of the document with the provided key. If there is
    /// no such element, the past-the-end iterator will be returned. The runtime of
    /// find() is linear in the length of the document. This method only searches
    /// the top-level document, and will not recurse to any subdocuments.
    ///
    /// @remark In BSON, keys are not required to be unique. If there are multiple
    /// elements with a matching key in the document, the first matching element from
    /// the start will be returned.
    ///
    /// @param key
    ///   The key to search for.
    ///
    /// @return An iterator to the matching element, if found, or the past-the-end iterator.
    ///
    BSONCXX_ABI_EXPORT_CDECL(document::view::const_iterator) find(stdx::string_view key) const;

    ///
    /// Finds the first element of the document with the provided key. If there is no
    /// such element, the invalid document::element will be returned. The runtime of operator[]
    /// is linear in the length of the document.
    ///
    /// @param key
    ///   The key to search for.
    ///
    /// @return The matching element, if found, or the invalid element.
    ///
    BSONCXX_ABI_EXPORT_CDECL(element) operator[](stdx::string_view key) const;

    ///
    /// Access the raw bytes of the underlying document.
    ///
    /// @return A pointer to the value's buffer.
    ///
    BSONCXX_ABI_EXPORT_CDECL(std::uint8_t const*) data() const;

    ///
    /// Gets the length of the underlying buffer.
    ///
    /// @remark This is not the number of elements in the document.
    /// To compute the number of elements, use std::distance.
    ///
    /// @return The length of the document, in bytes.
    ///
    BSONCXX_ABI_EXPORT_CDECL(std::size_t) length() const;

    ///
    /// Checks if the underlying document is empty, i.e. it is equivalent to
    /// the trivial document '{}'.
    ///
    /// @return true if the underlying document is empty.
    ///
    BSONCXX_ABI_EXPORT_CDECL(bool) empty() const;

    ///
    /// Get a view over the document owned by this value.
    ///
    document::view view() const noexcept {
        // Silence false positive with g++ 10.2.1 on Debian 11.
        BSONCXX_PRIVATE_WARNINGS_PUSH();
        BSONCXX_PRIVATE_WARNINGS_DISABLE(GCC("-Wmaybe-uninitialized"));
        return document::view{static_cast<uint8_t*>(_data.get()), _length};
        BSONCXX_PRIVATE_WARNINGS_POP();
    }

    ///
    /// Conversion operator that provides a view given a value.
    ///
    /// @return A view over the value.
    ///
    operator document::view() const noexcept {
        return view();
    }

    ///
    /// Constructs an object of type T from this document object. This method uses
    /// argument-dependent lookup to find the function declaration
    /// `void from_bson(T& t, bsoncxx::v_noabi::document::view const& doc)`.
    ///
    /// @note Type T must be default-constructible. Otherwise, use `void get(T& t)`.
    ///
    template <typename T>
    T get() {
        T temp{};
        from_bson(temp, this->view());
        return temp;
    }

    ///
    /// Constructs an object of type T from this document object. This method uses
    /// argument-dependent lookup to find the function declaration
    /// `void from_bson(T& t, bsoncxx::v_noabi::document::view const& doc)`.
    ///
    /// @param t
    ///   The object to construct. The contents of the document object will be deserialized
    ///   into t.
    ///
    template <typename T>
    void get(T& t) {
        from_bson(t, this->view());
    }

    ///
    /// Transfer ownership of the underlying buffer to the caller.
    ///
    /// @warning
    ///   After calling release() it is illegal to call any methods
    ///   on this class, unless it is subsequently moved into.
    ///
    /// @return A std::unique_ptr with ownership of the buffer.
    ///
    BSONCXX_ABI_EXPORT_CDECL(unique_ptr_type) release();

    ///
    /// Replace the formerly-owned buffer with the new view.
    /// This will make a copy of the passed-in view.
    ///
    BSONCXX_ABI_EXPORT_CDECL(void) reset(document::view view);

   private:
    unique_ptr_type _data;
    std::size_t _length{0};
};

///
/// Compares two document values for (in)-equality.
///
/// @{

/// @relatesalso bsoncxx::v_noabi::document::value
inline bool operator==(value const& lhs, value const& rhs) {
    return (lhs.view() == rhs.view());
}

/// @relatesalso bsoncxx::v_noabi::document::value
inline bool operator!=(value const& lhs, value const& rhs) {
    return !(lhs == rhs);
}

/// @}
///

} // namespace document
} // namespace v_noabi
} // namespace bsoncxx

namespace bsoncxx {
namespace document {

using ::bsoncxx::v_noabi::document::operator==;
using ::bsoncxx::v_noabi::document::operator!=;

} // namespace document
} // namespace bsoncxx

#include <bsoncxx/config/postlude.hpp>

///
/// @file
/// Provides @ref bsoncxx::v_noabi::document::value.
///
