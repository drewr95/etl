///\file

/******************************************************************************
The MIT License(MIT)

Embedded Template Library.
https://github.com/ETLCPP/etl
https://www.etlcpp.com

Copyright(c) 2014 John Wellbelove

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#ifndef ETL_SIGNAL_INCLUDED
#define ETL_SIGNAL_INCLUDED

#include "delegate.h"
#include "iterator.h"
#include "limits.h"
#include "memory.h"
#include "nullptr.h"
#include "platform.h"
#include "private/choose_namespace.h"
#include "reference_counted_object.h"
#include "type_traits.h"
#include "utility.h"
#include "vector.h"

#include <stddef.h>
#include <stdint.h>

//*****************************************************************************
/// Signals system based on https://github.com/fr00b0/nod.
//*****************************************************************************
namespace etl
{
  template <typename T>
  class isignal;

  namespace detail
  {
    class disconnector
    {
    public:
      virtual ~disconnector() ETL_NOEXCEPT = default;

      //***************************************************************************
      /// \brief Disconnects a slot at an index.
      /// \param index  Target slot position.
      //***************************************************************************
      virtual void operator()(size_t index) const ETL_NOEXCEPT = 0;
    };

    //***************************************************************************
    /// Base class for reference counted disconnectors.
    //***************************************************************************
    class ireference_counted_disconnector
    {
    public:
      virtual ~ireference_counted_disconnector() ETL_NOEXCEPT = default;
      ETL_NODISCARD virtual disconnector&                  get_disconnector() ETL_NOEXCEPT = 0;
      ETL_NODISCARD virtual const disconnector&            get_disconnector() const ETL_NOEXCEPT = 0;
      ETL_NODISCARD virtual etl::ireference_counter&       get_reference_counter() ETL_NOEXCEPT = 0;
      ETL_NODISCARD virtual const etl::ireference_counter& get_reference_counter() const ETL_NOEXCEPT = 0;
    };

    //***************************************************************************
    /// Reference counted disconnector implementation.
    //***************************************************************************
    template <typename TDisconnector>
    class reference_counted_disconnector final : public ireference_counted_disconnector
    {
    public:
      typedef TDisconnector disconnector_type;

      // Copies are forbidden.
      reference_counted_disconnector(const reference_counted_disconnector&) = delete;
      reference_counted_disconnector& operator=(const reference_counted_disconnector&) = delete;

      //***************************************************************************
      /// Constructor
      /// \param disconnector_  The disconnector.
      //***************************************************************************
      reference_counted_disconnector(const disconnector_type& disconnector_) ETL_NOEXCEPT
        : rc_disconnect{disconnector_}
      {
      }

      //***************************************************************************
      /// \return disconnector reference
      //***************************************************************************
      ETL_NODISCARD disconnector& get_disconnector() ETL_NOEXCEPT override
      {
        return rc_disconnect.get_object();
      }

      //***************************************************************************
      /// \return const disconnector reference
      //***************************************************************************
      ETL_NODISCARD const disconnector& get_disconnector() const ETL_NOEXCEPT override
      {
        return rc_disconnect.get_object();
      }

      //***************************************************************************
      /// \return reference counter reference
      //***************************************************************************
      ETL_NODISCARD etl::ireference_counter& get_reference_counter() ETL_NOEXCEPT override
      {
        return rc_disconnect.get_reference_counter();
      }

      //***************************************************************************
      /// \return Const reference counter reference
      //***************************************************************************
      ETL_NODISCARD const etl::ireference_counter& get_reference_counter() const ETL_NOEXCEPT override
      {
        return rc_disconnect.get_reference_counter();
      }

    private:
      etl::atomic_counted_object<disconnector_type> rc_disconnect;
    };

    //***************************************************************************
    /// Shares a disconnector.
    //***************************************************************************
    class weak_disconnector;
    class shared_disconnector final
    {
    public:
      shared_disconnector() ETL_NOEXCEPT = default;

      //***************************************************************************
      /// \brief Constructs a shared_disconnector.
      /// \param rcdisconnector_  The reference counted disconnector.
      //***************************************************************************
      shared_disconnector(ireference_counted_disconnector& rcdisconnector_) ETL_NOEXCEPT
        : p_rcdisconnector{&rcdisconnector_}
      {
        p_rcdisconnector->get_reference_counter().set_reference_count(1U);
      }

      //***************************************************************************
      /// \brief Constructs a shared_disconnector from a weak disconnector.
      /// \param weakDisconnector  The weak disconnector.
      //***************************************************************************
      shared_disconnector(weak_disconnector& weakDisconnector) ETL_NOEXCEPT;

      //***************************************************************************
      /// \brief Copy constructs a shared disconnector.
      /// \param other  The other shared disconnector.
      //***************************************************************************
      shared_disconnector(const shared_disconnector& other) ETL_NOEXCEPT
        : p_rcdisconnector{other.p_rcdisconnector}
      {
        increment_reference_count();
      }

#if ETL_USING_CPP11
      //***************************************************************************
      /// \brief Move constructs a shared disconnector.
      /// \param other  The other shared disconnector.
      //***************************************************************************
      shared_disconnector(shared_disconnector&& other) ETL_NOEXCEPT
        : p_rcdisconnector{ETL_OR_STD::exchange(other.p_rcdisconnector, nullptr)}
      {
      }
#endif // ETL_USING_CPP11

      //***************************************************************************
      /// \brief Destructor.
      //***************************************************************************
      ~shared_disconnector() ETL_NOEXCEPT
      {
        decrement_reference_count();
      }

      //***************************************************************************
      /// \brief Copy assignment.
      /// \param other  The other shared disconnector.
      /// \return *this  Shared disconnector.
      //***************************************************************************
      shared_disconnector& operator=(const shared_disconnector& other) ETL_NOEXCEPT
      {
        if(&other != this)
        {
          // Deal with the current disconnector.
          decrement_reference_count();

          // Copy over the new one.
          p_rcdisconnector = other.p_rcdisconnector;
          increment_reference_count();
        }
        return *this;
      }

      shared_disconnector& operator=(ireference_counted_disconnector& rcdisconnector_) ETL_NOEXCEPT
      {
        // Deal with the current disconnector
        decrement_reference_count();

        // Set the new one
        p_rcdisconnector = &rcdisconnector_;
        p_rcdisconnector->get_reference_counter().set_reference_count(1);
        return *this;
      }

#if ETL_USING_CPP11
      //***************************************************************************
      /// \brief Move assignment.
      /// \param other  The other shared disconnector.
      /// \return *this  Shared disconnector.
      //***************************************************************************
      shared_disconnector& operator=(shared_disconnector&& other) ETL_NOEXCEPT
      {
        if(&other != this)
        {
          // Deal with the current disconnector.
          decrement_reference_count();

          // Move over the new one.
          p_rcdisconnector = ETL_OR_STD::exchange(other.p_rcdisconnector, nullptr);
        }
        return *this;
      }
#endif // ETL_USING_CPP11

      //***************************************************************************
      /// \return Stored element if valid, nullptr otherwise.
      //***************************************************************************
      ETL_NODISCARD disconnector* get() ETL_NOEXCEPT
      {
        return is_valid() ? &p_rcdisconnector->get_disconnector() : nullptr;
      }

      //***************************************************************************
      /// \brief Const stored element if valid, nullptr otherwise.
      //***************************************************************************
      ETL_NODISCARD const disconnector* get() const ETL_NOEXCEPT
      {
        return is_valid() ? &p_rcdisconnector->get_disconnector() : nullptr;
      }

      //***************************************************************************
      /// \return Dereferenced result of get().
      //***************************************************************************
      ETL_NODISCARD disconnector& operator*() ETL_NOEXCEPT
      {
        return *get();
      }

      //***************************************************************************
      /// \return Dereferenced result of get().
      //***************************************************************************
      ETL_NODISCARD const disconnector& operator*() const ETL_NOEXCEPT
      {
        return *get();
      }

      //***************************************************************************
      /// \return Pointer to disconnector if valid, otherwise nullptr.
      //***************************************************************************
      ETL_NODISCARD disconnector* operator->() ETL_NOEXCEPT
      {
        return get();
      }

      //***************************************************************************
      /// \return Pointer to disconnector if valid, otherwise nullptr.
      //***************************************************************************
      ETL_NODISCARD const disconnector* operator->() const ETL_NOEXCEPT
      {
        return get();
      }

      //***************************************************************************
      /// \return  The amount of shared_disconnectors managing the object.
      /// \return  0 if the disconnector is invalid.
      //***************************************************************************
      ETL_NODISCARD int32_t use_count() const ETL_NOEXCEPT
      {
        if (p_rcdisconnector != ETL_NULLPTR)
        {
          return p_rcdisconnector->get_reference_counter().get_reference_count();
        }
        return 0;
      }

      //***************************************************************************
      /// \return true  If vallid.
      //***************************************************************************
      ETL_NODISCARD bool is_valid() const ETL_NOEXCEPT
      {
        return use_count() > 0;
      }

      //***************************************************************************
      /// \return true  If valid.
      //***************************************************************************
      ETL_NODISCARD operator bool() const ETL_NOEXCEPT
      {
        return is_valid();
      }

    private:
      friend class weak_disconnector;
      ireference_counted_disconnector* p_rcdisconnector{nullptr};

      //***************************************************************************
      /// \brief Handles incrementing the reference count.
      //***************************************************************************
      void increment_reference_count() ETL_NOEXCEPT
      {
        if(p_rcdisconnector != ETL_NULLPTR)
        {
          p_rcdisconnector->get_reference_counter().increment_reference_count();
        }
      }

      //***************************************************************************
      /// \brief Handles decrementing the reference count.
      //***************************************************************************
      void decrement_reference_count() ETL_NOEXCEPT
      {
        if(is_valid())
        {
          if(p_rcdisconnector->get_reference_counter().decrement_reference_count() == 0)
          {
            p_rcdisconnector = ETL_NULLPTR;
          }
        }
      }
    };

    //***************************************************************************
    /// Shares a disconnector.
    //***************************************************************************
    class weak_disconnector final
    {
    public:
      weak_disconnector() ETL_NOEXCEPT = default;
      ~weak_disconnector() ETL_NOEXCEPT = default;
      weak_disconnector(const weak_disconnector& other) ETL_NOEXCEPT = default;

#if ETL_USING_CPP11
      weak_disconnector(weak_disconnector&& other) ETL_NOEXCEPT = default;
      weak_disconnector& operator=(weak_disconnector&& other) ETL_NOEXCEPT = default;
#endif // ETL_USING_CPP11

      //***************************************************************************
      /// \brief  Constructs weak_disconnector from a shared_disconnector.
      //***************************************************************************
      weak_disconnector(shared_disconnector& sharedDisconnector) ETL_NOEXCEPT
        : p_rcdisconnector{sharedDisconnector.p_rcdisconnector}
      {
      }

      //***************************************************************************
      /// \return  The amount of shared_disconnectors managing the object.
      /// \return  0 if the disconnector is invalid.
      //***************************************************************************
      ETL_NODISCARD int32_t use_count() const ETL_NOEXCEPT
      {
        if(p_rcdisconnector != ETL_NULLPTR)
        {
          return p_rcdisconnector->get_reference_counter().get_reference_count();
        }
        return 0;
      }

      //***************************************************************************
      /// \return  True if there are no shared_disconnectors managing the object.
      //***************************************************************************
      ETL_NODISCARD bool is_expired() const ETL_NOEXCEPT
      {
        return use_count() == 0;
      }

      //***************************************************************************
      /// \return  Shared disconnector from this weak_disconnector.
      //***************************************************************************
      ETL_NODISCARD shared_disconnector lock() ETL_NOEXCEPT
      {
        return is_expired() ? shared_disconnector{} : shared_disconnector{*this};
      }

      //***************************************************************************
      /// \brief Sets the pointer to the reference counted disconnector to nullptr.
      //***************************************************************************
      void reset() ETL_NOEXCEPT
      {
        p_rcdisconnector = ETL_NULLPTR;
      }

      //***************************************************************************
      /// \brief  Copy assignment from another shared_disconnector.
      /// \return  *this weak_disconnector.
      //***************************************************************************
      weak_disconnector& operator=(const shared_disconnector& other) ETL_NOEXCEPT
      {
        p_rcdisconnector = other.p_rcdisconnector;
        return *this;
      }

    private:
      friend class shared_disconnector;
      ireference_counted_disconnector* p_rcdisconnector{nullptr};
    };

    inline shared_disconnector::shared_disconnector(weak_disconnector& weakDisconnector) ETL_NOEXCEPT
      : p_rcdisconnector{weakDisconnector.p_rcdisconnector}
    {
      increment_reference_count();
    }
  }  // namespace detail

  //*************************************************************************
  /// A connection to a signal.
  //*************************************************************************
  class connection final
  {
  public:
    connection() ETL_NOEXCEPT = default;
    ~connection() ETL_NOEXCEPT = default;

    // copy constructing and assignment is prohibited
    connection(const connection&) = delete;
    connection& operator=(const connection&) = delete;

    //***************************************************************************
    /// \brief Move constructs an object.
    /// \param other  To move into this.
    //***************************************************************************
    connection(connection&& other) ETL_NOEXCEPT
      : weak_disconnect{ETL_OR_STD::exchange(other.weak_disconnect, {})},
        index{ETL_OR_STD::exchange(other.index, 0U)}
    {
    }

    //***************************************************************************
    /// \brief Move assigns another object to this.
    /// \param other  Object to move.
    //***************************************************************************
    connection& operator=(connection&& other) ETL_NOEXCEPT
    {
      if (&other != this)
      {
        weak_disconnect = ETL_OR_STD::exchange(other.weak_disconnect, {});
        index = ETL_OR_STD::exchange(other.index, 0U);
      }
      return *this;
    }

    //***************************************************************************
    /// \return true  If connected.
    //***************************************************************************
    ETL_NODISCARD bool is_connected() const ETL_NOEXCEPT
    {
      return !weak_disconnect.is_expired();
    }

    //***************************************************************************
    /// \brief Disconnects the slot from the connection.
    //***************************************************************************
    void disconnect() noexcept;

  private:
    template <typename T>
    friend class isignal;

    etl::detail::weak_disconnector weak_disconnect{};
    size_t                         index{0U};

    //***************************************************************************
    /// Constructor
    /// \param shared_disconnect_  Shared disconnector reference.
    /// \param index_  Index to the slot.
    //***************************************************************************
    connection(
      etl::detail::shared_disconnector& shared_disconnect_,
      const size_t                      index) ETL_NOEXCEPT
      : weak_disconnect{shared_disconnect_}
      , index{index}
    {
    }
  };

  //*************************************************************************
  /// A connection to a signal that disconnects when out of scope.
  //*************************************************************************
  class scoped_connection final
  {
  public:
    // copy constructing and assignment is prohibited
    scoped_connection(const scoped_connection& other) = delete;
    scoped_connection& operator=(const scoped_connection& other) = delete;

    scoped_connection() ETL_NOEXCEPT = default;

    //***************************************************************************
    /// \brief  Move constructs a connection to this.
    /// \param c  Connection to move.
    //***************************************************************************
    scoped_connection(connection&& c) ETL_NOEXCEPT
      : conn{ETL_OR_STD::forward<connection>(c)}
    {
    }

    //***************************************************************************
    /// \brief  Disconnects the slot from the signal.
    //***************************************************************************
    ~scoped_connection() ETL_NOEXCEPT
    {
      disconnect();
    }

    //***************************************************************************
    /// \brief Move assigns a connection to this.
    /// \param other  Object to move.
    /// \return this
    //***************************************************************************
    scoped_connection(scoped_connection&& other) ETL_NOEXCEPT
      : conn{ETL_OR_STD::exchange(other.conn, {})}
    {
    }

    //***************************************************************************
    /// \brief Move assigns a connection to this.
    /// \param other  Object to move.
    /// \return this
    //***************************************************************************
    scoped_connection& operator=(connection&& other) ETL_NOEXCEPT
    {
      reset(ETL_OR_STD::forward<connection>(other));
      return *this;
    }

    //***************************************************************************
    /// \brief Move assigns another scoped_connection to this.
    /// \param other  Object to move.
    /// \return this
    //***************************************************************************
    scoped_connection& operator=(scoped_connection&& other) ETL_NOEXCEPT
    {
      reset(ETL_OR_STD::exchange(other.conn, {}));
      return *this;
    }

    //***************************************************************************
    /// \brief Resets the connection.
    //***************************************************************************
    void reset(connection&& c = {}) ETL_NOEXCEPT
    {
      disconnect();
      conn = ETL_OR_STD::move(c);
    }

    //***************************************************************************
    /// \brief Releases the current connection.
    /// \return Current connection.
    //***************************************************************************
    ETL_NODISCARD connection release() ETL_NOEXCEPT
    {
      auto c = ETL_OR_STD::exchange(conn, {});
      conn = connection{};
      return c;
    }

    //***************************************************************************
    /// \brief Determines if a connection is connected.
    /// \return true  If connected.
    //***************************************************************************
    ETL_NODISCARD bool is_connected() const ETL_NOEXCEPT
    {
      return conn.is_connected();
    }

    //***************************************************************************
    /// \brief Disconnects the slot from the signal.
    //***************************************************************************
    void disconnect() noexcept
    {
      conn.disconnect();
    }

  private:
    connection conn;
  };

  //*************************************************************************
  /// Base signal class.
  //*************************************************************************
  template <typename TReturn, typename... TArgs>
  class isignal<TReturn(TArgs...)>
  {
  public:
    typedef etl::delegate<TReturn(TArgs...)>   slot_type;
    typedef typename etl::ivector<slot_type>::size_type size_type;

    //***************************************************************************
    /// \brief  Invalidates the disconnector of this signal when destructed.
    //***************************************************************************
    ~isignal() ETL_NOEXCEPT
    {
      invalidate_disconnector();
    }

    //*************************************************************************
    /// \brief Connects a slot to the signal.
    /// \tparam TSlot  Slot type.
    /// \param slot  To connect.
    /// \return A connection to the signal.
    //*************************************************************************
    template <typename TSlot>
    connection connect(TSlot&& slot) ETL_NOEXCEPT
    {
      p_slots->push_back(ETL_OR_STD::forward<TSlot>(slot));
      const size_type index = p_slots->size() - 1U;
      ++slot_count;
      return connection{shared_disconnect, index};
    }

    //*************************************************************************
    /// \brief Connects a free function slot.
    /// \tparam (*Method)(TArgs...)  Free function.
    /// \return A connection to the signal.
    //*************************************************************************
    template <TReturn (*Method)(TArgs...)>
    connection connect() ETL_NOEXCEPT
    {
      return connect(slot_type::template create<Method>());
    }

    //*************************************************************************
    /// \brief Connects a compile-time known class method.
    /// \tparam T  Class type.
    /// \tparam (T::Method)(TArgs...)  Class method.
    /// \param object  Object instance
    /// \return A connection to the signal.
    //*************************************************************************
    template <typename T, TReturn (T::*Method)(TArgs...)>
    connection connect(T& object) ETL_NOEXCEPT
    {
      return connect(slot_type::template create<T, Method>(object));
    }

    //*************************************************************************
    /// \brief Connects a compile-time known class method.
    /// \tparam T  Class type.
    /// \tparam (T::Method)(TArgs...)  Class method.
    /// \param object  Object instance
    /// \return A connection to the signal.
    //*************************************************************************
    template <typename T, TReturn (T::*Method)(TArgs...)>
    connection connect(T* const object) ETL_NOEXCEPT
    {
      return connect<T, Method>(*object);
    }

    //*************************************************************************
    /// \brief Handles triggering the slots.
    /// \param args  Arguments to propagate to the slots.
    //*************************************************************************
    void operator()(const TArgs&... args) const ETL_NOEXCEPT
    {
      for (const auto& slot : *p_slots)
      {
        slot.call_if(std::forward<const TArgs&>(args)...);
      }
    }

    //*************************************************************************
    /// \return The number of connected slots.
    //*************************************************************************
    ETL_NODISCARD size_type count() const ETL_NOEXCEPT
    {
      return slot_count;
    }

    //*************************************************************************
    /// \return max slots
    //*************************************************************************
    ETL_NODISCARD size_type capacity() const ETL_NOEXCEPT
    {
      return p_slots->capacity();
    }

    //*************************************************************************
    /// \return The number of available slots left.
    //*************************************************************************
    ETL_NODISCARD size_type available() const ETL_NOEXCEPT
    {
      return capacity() - count();
    }

    //*************************************************************************
    /// \return true  If there are no slots connected.
    //*************************************************************************
    ETL_NODISCARD bool is_empty() const ETL_NOEXCEPT
    {
      return p_slots->empty();
    }

    //*************************************************************************
    /// \return true  If there are no available slots left.
    //*************************************************************************
    ETL_NODISCARD bool is_full() const ETL_NOEXCEPT
    {
      return p_slots->full();
    }

    //*************************************************************************
    /// \brief Disconnects all slots.
    //*************************************************************************
    void disconnect_all_slots() ETL_NOEXCEPT
    {
      p_slots->clear();
      slot_count = 0;
      invalidate_disconnector();
    }

  protected:
    //*************************************************************************
    /// \brief  Constructs an isignal.
    /// \param slots_  Slots to set.
    //*************************************************************************
    isignal(etl::ivector<slot_type>& slots_) ETL_NOEXCEPT
      : p_slots{&slots_}
    {
    }

#if ETL_USING_CPP11
    //*************************************************************************
    /// \brief  Move constructs an isignal.
    /// \param other  The other signal to move to this.
    //*************************************************************************
    isignal(isignal&& other) ETL_NOEXCEPT
      : p_slots{ETL_OR_STD::exchange(other.p_slots, ETL_NULLPTR)}
      , slot_count{ETL_OR_STD::exchange(other.slot_count, 0U)}
    {
      update_disconnectors(std::move(other));
    }

    //*************************************************************************
    /// \brief  Move assigns an isignal.
    /// \param other  The other signal to move to this.
    //*************************************************************************
    isignal& operator=(isignal&& other) noexcept
    {
      p_slots = ETL_OR_STD::exchange(other.p_slots, ETL_NULLPTR);
      slot_count = ETL_OR_STD::exchange(other.slot_count, 0U);
      update_disconnectors(std::move(other));
      return *this;
    }
#endif // ETL_USING_CPP11

    //*************************************************************************
    /// \brief Sets the slots.
    /// \param slots_  Slots to set.
    //*************************************************************************
    void set_slots(etl::ivector<slot_type>& slots_) ETL_NOEXCEPT
    {
      p_slots = &slots_;
    }

  private:
    class disconnector final : public detail::disconnector
    {
    public:
      typedef isignal<TReturn(TArgs...)> signal_type;

      disconnector() ETL_NOEXCEPT = default;
      ~disconnector() ETL_NOEXCEPT override = default;

      //*************************************************************************
      /// \brief Constructs a disconnector from a signal pointer
      /// \param ptr_  Pointer to signal.
      //*************************************************************************
      explicit disconnector(signal_type* ptr_) ETL_NOEXCEPT
        : p_signal{ptr_}
      {
      }

      //*************************************************************************
      /// \brief Disconnects a slot at index for the current signal.
      /// \param index  Index of the slot to disconnect.
      //*************************************************************************
      void operator()(const size_t index) const ETL_NOEXCEPT override
      {
        if (p_signal != ETL_NULLPTR)
        {
          p_signal->disconnect_slot(index);
        }
      }

    private:
      signal_type* p_signal{nullptr};
    };

    typedef etl::detail::reference_counted_disconnector<disconnector> rc_disconnector;
    typedef etl::detail::shared_disconnector                          shared_disconnector;

    etl::ivector<slot_type>*    p_slots;
    size_type                   slot_count{0};
    rc_disconnector             rc_disconnect{disconnector{this}};
    detail::shared_disconnector shared_disconnect{rc_disconnect};

    //*************************************************************************
    /// \brief Updates the disconnectors.
    /// \param other  Other signal moving to this.
    //*************************************************************************
    void update_disconnectors(isignal&& other) ETL_NOEXCEPT
    {
      if(other.shared_disconnect)
      {
        /// \note Set the other shared disconnect to point to our disconnector
        /// to transfer the other's weak disconnectors. This is essentially moving
        /// the existing connections to this signal.
        other.shared_disconnect = shared_disconnect;
      }
    }

    //*************************************************************************
    /// \brief  Invalidates the disconnector.
    //*************************************************************************
    void invalidate_disconnector() ETL_NOEXCEPT
    {
      /// \note This invalides the disconnector by setting the reference counted 
      /// disconnector to nullptr. When a connection disconnects, the disconnector
      /// will check if the signal is valid before disconnecting the slot.  Since 
      /// the signal is now nullptr, the disconnector will do nothing.
      rc_disconnect.get_disconnector() = disconnector{};
    }

    //*************************************************************************
    /// \brief Disconnects a slot at an index.
    /// \param index  Target slot position.
    //*************************************************************************
    void disconnect_slot(const size_t index)
    {
      if ((*p_slots)[index])
      {
        --slot_count;
      }
      (*p_slots)[index] = slot_type{};
      while ((p_slots->size() > 0) && (!p_slots->back()))
      {
        p_slots->pop_back();
      }
    }
  };

  template <typename T, size_t SIZE_>
  class signal final : public isignal<T>
  {
  public:
    typedef isignal<T> base_type;
    using typename base_type::size_type;
    using typename base_type::slot_type;

    static constexpr size_type SIZE{SIZE_};

    // Copy construction and assignment is forbidden.
    signal(const signal&) = delete;
    signal& operator=(const signal&) = delete;

    ~signal() ETL_NOEXCEPT = default;

    //*************************************************************************
    /// \brief Constructor
    //*************************************************************************
    signal() ETL_NOEXCEPT
      : base_type{slots}
    {
    }

    //*************************************************************************
    /// \brief Move constructs a signal.
    /// \param other  The other signal.
    //*************************************************************************
    signal(signal&& other) ETL_NOEXCEPT
      : base_type{ETL_OR_STD::move(other)}
    {
      slots = ETL_OR_STD::exchange(other.slots, {});
      base_type::set_slots(slots);
    }

    //*************************************************************************
    /// \brief Move assigns another signal.
    /// \param other  The other signal.
    /// \return this
    //*************************************************************************
    signal& operator=(signal&& other) noexcept
    {
      if (&other != this)
      {
        base_type::operator=(ETL_OR_STD::move(other));
        slots = ETL_OR_STD::exchange(other.slots, {});
        base_type::set_slots(slots);
      }
      return *this;
    }

  private:
    etl::vector<slot_type, SIZE> slots;
  };

  //*************************************************************************
  /// \brief  Disconnects a slot (at index) from the signal.
  //*************************************************************************
  inline void connection::disconnect() noexcept
  {
    auto ptr = weak_disconnect.lock();
    if(ptr)
    {
      (*ptr)(index);
    }
    weak_disconnect.reset();
  }

}  // namespace etl

#endif
