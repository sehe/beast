//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

// Test that header file is self-contained.
#include <boost/beast/core/string.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <boost/core/detail/string_view.hpp>
#include <boost/utility/string_view.hpp>

#include <ostream>
#include <istream>
#include <iostream> // TODO REMOVE
#if __has_include(<version>)
    #include <version> // for feature test macros to be reliable
    #ifdef __cpp_lib_string_view
        #define BEAST_TEST_HAVE_STD_STRING_VIEW 1
    #else
        #define BEAST_TEST_HAVE_STD_STRING_VIEW 0
    #endif
#else
    #pragma message "no reliable feature test macros"
    #ifdef _LIBCPP_VERSION
        // TODO FIXME forcing this for libc++
        #define BEAST_TEST_HAVE_STD_STRING_VIEW 1
    #endif
#endif

#ifndef BEAST_TEST_HAVE_STD_STRING_VIEW
    #define BEAST_TEST_HAVE_STD_STRING_VIEW 0
    #pragma message "no std::string_view implementation detected"
#endif

#if BEAST_TEST_HAVE_STD_STRING_VIEW
    #include <string_view>
#elif defined(__cpp_lib_experimental_string_view)
    #include <experimental/string_view>
#endif

namespace boost {
namespace beast {

namespace test_detail {
namespace string_views {
namespace /*anonymous*/ {
template <typename, template <typename...> typename>
struct is_instance_impl : public std::false_type {};

template <template <typename...> typename U, typename... Ts>
struct is_instance_impl<U<Ts...>, U> : public std::true_type {};
} // namespace

template <typename T, template <typename...> typename U>
static inline constexpr bool is_instance_v = is_instance_impl<std::decay_t<T>, U>::value;

template <typename CharT> struct Fixture {
    static constexpr CharT const empty[] = "";
    static constexpr CharT const sz1234[] = "1234";
    static constexpr size_t len = std::size(sz1234) - 1; // not including NUL
};

template <> struct Fixture<wchar_t> {
    static constexpr wchar_t const empty[] = L"";
    static constexpr wchar_t const sz1234[] = L"1234";
    static constexpr size_t len = std::size(sz1234) - 1; // not including NUL
};

template <> struct Fixture<char16_t> {
    static constexpr char16_t const empty[] = u"";
    static constexpr char16_t const sz1234[] = u"1234";
    static constexpr size_t len = std::size(sz1234) - 1; // not including NUL
};

template <> struct Fixture<char32_t> {
    static constexpr char32_t const empty[] = U"";
    static constexpr char32_t const sz1234[] = U"1234";
    static constexpr size_t len = std::size(sz1234) - 1; // not including NUL
};

template <typename SV, typename CharT> struct CheckInstance {
    static constexpr Fixture<CharT> fixture{};
    static_assert(fixture.len == 4);

    using traits_type            = typename SV::traits_type;
    using iterator               = typename SV::iterator;
    using const_iterator         = typename SV::const_iterator;
    using reverse_iterator       = typename SV::reverse_iterator;
    using const_reverse_iterator = typename SV::const_reverse_iterator;
    using value_type             = typename SV::value_type;
    using pointer                = typename SV::pointer;
    using const_pointer          = typename SV::const_pointer;
    using reference              = typename SV::reference;
    using const_reference        = typename SV::const_reference;
    using size_type              = typename SV::size_type;
    using difference_type        = typename SV::difference_type;

    static_assert(std::is_same_v<CharT const*, const_pointer>);
    static_assert(std::is_same_v<CharT const*, const_iterator>);
    static_assert(std::is_same_v<CharT, value_type>);
    // static_assert(std::is_same_v<CharT const*, pointer>); // OBSERVABLE
    // static_assert(std::is_same_v<CharT&, reference>); // OBSERVABLE
    static_assert(std::is_same_v<CharT const&, const_reference>);

    static_assert(not std::is_aggregate_v<SV>);
    static_assert(not std::is_trivial_v<SV>);
    static_assert(not std::is_trivially_constructible_v<SV>);
    static_assert(not std::is_trivially_default_constructible_v<SV>);
    static_assert(std::is_trivially_copy_constructible_v<SV>);
    static_assert(std::is_trivially_move_constructible_v<SV>);
    static_assert(std::is_trivially_copy_assignable_v<SV>);
    static_assert(std::is_trivially_move_assignable_v<SV>);
    static_assert(std::is_trivially_copyable_v<SV>);
    static_assert(std::is_trivially_destructible_v<SV>);
    static_assert(std::is_standard_layout_v<SV>);
    static_assert(sizeof(SV) == 2*sizeof(size_t));

    static_assert(std::is_same_v<std::size_t, size_type>);
    static_assert(std::is_same_v<std::ptrdiff_t, difference_type>);

    static_assert(std::is_same_v<std::reverse_iterator<iterator>, reverse_iterator>);
    static_assert(std::is_same_v<std::reverse_iterator<const_iterator>, const_reverse_iterator>);
    static_assert(std::is_same_v<std::ptrdiff_t, difference_type>);

    static_assert(std::is_same_v<std::decay_t<decltype(SV::npos)>, size_type>);
    static_assert(SV::npos == size_type(-1));

    // make sure template instantiation is mentioned at failure
#define LOCAL_EXPECT(cond) BEAST_EXPECTS(cond, __PRETTY_FUNCTION__)

    void run() {
        // member functions
        /// constructors, iterators and assignment
        check_empty_instances();
        check_non_empty_instances();

        check_constructors();

        check_copy_and_assign();

        /// element access
        check_element_access();

        /// capacity
        check_capacity();

        /// modifiers
        check_modifiers();

        /// operations
        check_operations();

        // non-member
        check_relational();
        check_hashing();

        // interface usage
        check_argument_passing();

        
    };

private:
    void check_element_access() {
        {
            // NOTE (sehe) assuming sane implementations match non-const behaviour
            SV const instance{fixture.sz1234};

            for (size_t i = 0; i < instance.length(); ++i)
                LOCAL_EXPECT(instance[i] == fixture.sz1234[i]);

            for (size_t i = 0; i < instance.length(); ++i)
                LOCAL_EXPECT(instance.at(i) == fixture.sz1234[i]);

            LOCAL_EXPECT(instance.front() == instance[0]);
            LOCAL_EXPECT(std::addressof(instance.front()) == std::addressof(instance[0]));
            LOCAL_EXPECT(std::addressof(instance.front()) == instance.data());

            LOCAL_EXPECT(instance.back() == instance[instance.length() - 1]);
            LOCAL_EXPECT(std::addressof(instance.back()) == std::addressof(instance[instance.length() - 1]));
        }

        { 
            SV const instance{fixture.sz1234, fixture.len - 1};

#ifndef NDEBUG
            // skipped because implementations may assert
#else
            instance[instance.length()]; // doesn't throw, also no UB due
                                         // underlying fixture::sz1234
#endif

            BEAST_THROWS(instance.at(instance.length()), std::out_of_range);
        }
    }

    void check_capacity() {
        // length(), size(), empty(): see
        // check_non_empty_instances/check_empty_instances
        check_capacity_max_size(); // this is its own saga...
    }

    void check_capacity_max_size() {
        static_assert(
            std::is_same_v<decltype(std::declval<SV>().max_size()), size_t>);

        // ¯\_(ツ)_/¯ either `max_size()` returns exactly `length()` OR is some
        // implementation-defined Very Big Value
        LOCAL_EXPECT(SV{fixture.sz1234}.max_size() == fixture.len ||
                     SV{}.max_size() >= 0x0fff'ffff'ffff'fff0ULL);

        // Subsequent static_assert made into LOCAL_EXPECT for easy debugging

        // OBSERVABLE differences:
        if constexpr (is_instance_v<SV, boost::basic_string_view>) {
            // boost::basic_string_view erroneously returns max_size() as
            // length()
            LOCAL_EXPECT(0 == SV{}.max_size()); // oops
#if BEAST_TEST_HAVE_STD_STRING_VIEW
        } else if constexpr (is_instance_v<SV, std::basic_string_view>) {
    #ifdef _LIBCPP_VERSION
            LOCAL_EXPECT(SV{}.max_size() == size_t(-1));
    #else // libstdc++ assumed:
            LOCAL_EXPECT(SV{}.max_size() ==
                         (std::numeric_limits<difference_type>::max() - 8) /
                             (2 * sizeof(CharT)));
    #endif
#else
            LOCAL_EXPECT(!"std::basic_string_view not applicable");
#endif
        } else {
            // size/length see check_non_empty_instances/check_empty_instances
            LOCAL_EXPECT(SV{}.max_size() ==
                          (std::numeric_limits<size_t>::max()) /
                              (sizeof(CharT)));
        }
    }

    void check_modifiers() { // TODO SEHE
        // remove_prefix
        {
            auto removed = [](size_t n) {
                SV sv{fixture.sz1234, fixture.len};
                sv.remove_prefix(n);
                return sv;
            };
            //// OBSERVABLE only boost::utility::basic_string_view allows this, others assert
            //UB: LOCAL_EXPECT(removed(-1) == fixture.empty);
            LOCAL_EXPECT(removed(0) == fixture.sz1234 + 0);
            LOCAL_EXPECT(removed(1) == fixture.sz1234 + 1);
            LOCAL_EXPECT(removed(2) == fixture.sz1234 + 2);
            LOCAL_EXPECT(removed(3) == fixture.sz1234 + 3);
        }
        // remove_suffix
        {
            auto removed = [](size_t n) {
                SV sv{fixture.sz1234, fixture.len};
                sv.remove_suffix(n);
                return sv;
            };
            //// OBSERVABLE only boost::utility::basic_string_view allows this, others
            //assert (GNU libstdc++ doesn't, even though remove_prefix does...)
            //UB: LOCAL_EXPECT(removed(-1) == fixture.empty);
            LOCAL_EXPECT((removed(0) == SV{fixture.sz1234, 4}));
            LOCAL_EXPECT((removed(1) == SV{fixture.sz1234, 3}));
            LOCAL_EXPECT((removed(2) == SV{fixture.sz1234, 2}));
            LOCAL_EXPECT((removed(3) == SV{fixture.sz1234, 1}));
        }
        // swap
        {
            SV a{fixture.sz1234}, b{fixture.empty};
            {
                // ADL swap
                using std::swap;
                swap(a, b);
            }

            LOCAL_EXPECT(a == fixture.empty);
            LOCAL_EXPECT(b == fixture.sz1234);

            a.swap(b);

            LOCAL_EXPECT(a == fixture.sz1234);
            LOCAL_EXPECT(b == fixture.empty);
        }
    }

    static bool constexpr is_std_basic_string_view =
#if BEAST_TEST_HAVE_STD_STRING_VIEW
        is_instance_v<SV, std::basic_string_view>
#else
        false
#endif
        ;

    static bool constexpr skip_starts_with_ends_with = is_std_basic_string_view
#ifdef __cpp_lib_starts_ends_with
        && (__cpp_lib_starts_ends_with < 201711)
#endif
        ;

    static bool constexpr skip_contains =
        // OBSERVABLE: boost::basic_string_view doesn't have `contains`
        is_instance_v<SV, boost::basic_string_view> ||
        (is_std_basic_string_view
#ifdef __cpp_lib_string_contains
         && (__cpp_lib_string_contains < 202011)
#endif
        );

    void check_operations() { // TODO SEHE
        // copy
        {
            CharT buf[fixture.len] = {};
            SV bufvw{buf, std::size(buf)};

            SV const sv{fixture.sz1234};

            std::fill(std::begin(buf), std::end(buf), CharT{});
            sv.copy(buf, fixture.len, 0);
            LOCAL_EXPECT(sv == bufvw);

            std::fill(std::begin(buf), std::end(buf), CharT{});
            sv.copy(buf, fixture.len); // pos = 0 default argument
            LOCAL_EXPECT(sv == bufvw);

            sv.copy(buf+2, fixture.len-2);
            auto front = bufvw.substr(0, 2), back = bufvw.substr(2);
            LOCAL_EXPECT(front == back);

            LOCAL_EXPECT(bufvw[3] == sv[1]); // pre-condition

            sv.copy(buf + 1, fixture.len, 2); // count gets clamped to 2
            LOCAL_EXPECT(bufvw[0] == sv[0]);
            LOCAL_EXPECT(bufvw[1] == sv[2]);
            LOCAL_EXPECT(bufvw[2] == sv[3]);
            LOCAL_EXPECT(bufvw[3] == sv[1]); // not clobbered by last copy

            std::fill(std::begin(buf), std::end(buf), CharT{});
            sv.copy(buf, 100, fixture.len); // starting at the end is noop

            LOCAL_EXPECT(fixture.len == std::count(std::begin(buf), std::end(buf), CharT{}));
            BEAST_THROWS(sv.copy(buf, 0, fixture.len+1), std::out_of_range);
        }
        // substr
        {
            SV const sv{fixture.sz1234};

            // OBSERVABLE: utility::basic_string_view doesn't default pos=0
            if constexpr (not is_instance_v<SV, boost::basic_string_view>) {
                LOCAL_EXPECT(sv == sv.substr());
            }

            LOCAL_EXPECT(sv.substr(1).length() == sv.length() - 1);
            LOCAL_EXPECT(sv.substr(2).length() == sv.length() - 2);
            LOCAL_EXPECT(sv.substr(3).length() == sv.length() - 3);
            LOCAL_EXPECT(sv.substr(4).length() == sv.length() - 4);
            LOCAL_EXPECT(sv.substr(4).empty());
            LOCAL_EXPECT(sv.substr(1, 2).length() == 2);
            LOCAL_EXPECT(sv.substr(1, 15).length() == 3); // count clamped

            BEAST_THROWS(sv.substr(5), std::out_of_range);
            BEAST_THROWS(sv.substr(5, 0), std::out_of_range);
        }
        // compare
        {
            SV const sv{fixture.sz1234};

            CharT buf[fixture.len*4] = {};
            SV bufvw{buf, std::size(buf)};

            for (CharT* cur = buf; cur < std::end(buf); cur += fixture.len) {
                sv.copy(cur, fixture.len);
            }

            LOCAL_EXPECT(sv.compare(bufvw) < 0);
            LOCAL_EXPECT(sv.compare(bufvw.data()) < 0); // CharT const* rhs
            LOCAL_EXPECT(bufvw.compare(sv) > 0);
            LOCAL_EXPECT(bufvw.substr(0, sv.length()).compare(sv) == 0);
            LOCAL_EXPECT(bufvw.substr(0, sv.length()-1).compare(sv) < 0);

            for (size_t i = 0; i < std::size(buf); ++i) {
                for (auto n : {2,4,8,16}) {
                    LOCAL_EXPECT(bufvw.compare(i, n, sv) ==
                                 bufvw.substr(i, n).compare(sv));
                    LOCAL_EXPECT(bufvw.compare(i, n, sv.data()) ==
                                 bufvw.substr(i, n).compare(sv));
                    if (i < sv.length()) {
                        LOCAL_EXPECT(
                            bufvw.compare(i, n, sv, i, n) ==
                            bufvw.substr(i, n).compare(sv.substr(i, n)));
                        LOCAL_EXPECT(
                            bufvw.compare(i, n, sv.data(), i, n) ==
                            bufvw.substr(i, n).compare(sv.substr(i, n)));
                    }
                }
            }

            if constexpr (is_instance_v<SV, boost::basic_string_view>) {
                // OBSERVABLE:
                // overloads on Boost Utility's boost::basic_string_view are
                // supposed to propagate std::out_of_range from substr, but are
                // mistakenly marked noexcept. Oops. That crashes.
            } else {
                BEAST_THROWS(sv.compare(5, 0, bufvw), std::out_of_range);
            }
        }

        // starts_with
        if constexpr (!skip_starts_with_ends_with) {
            SV const sv{fixture.sz1234};

            CharT buf[fixture.len*4] = {};
            SV bufvw{buf, std::size(buf)};

            for (CharT* cur = buf; cur < std::end(buf); cur += fixture.len) {
                sv.copy(cur, fixture.len);
            }

            LOCAL_EXPECT(bufvw.starts_with(sv));
            LOCAL_EXPECT(bufvw.starts_with(sv.data())); // assume NUL-termination
            LOCAL_EXPECT(not sv.starts_with(bufvw)); // sv shorter
            LOCAL_EXPECT(sv.starts_with(bufvw.front())); // single char
            LOCAL_EXPECT(not SV{}.starts_with(bufvw.front())); // SV{} empty

            for (size_t i = 0; i <= std::size(buf); ++i) {
                LOCAL_EXPECT(
                    bufvw.substr(i).starts_with(sv) ==
                    ((i < std::size(buf)) && (0 == (i % sv.length()))));
            }
            for (size_t n = 0; n < std::size(buf); ++n) {
                LOCAL_EXPECT(bufvw.substr(0, n).starts_with(sv) ==
                             (n >= sv.length()));
            }
        }

        // ends_with
        if constexpr (!skip_starts_with_ends_with) {
            SV const sv{fixture.sz1234};

            CharT buf[fixture.len*4] = {};
            SV bufvw{buf, std::size(buf)};

            for (CharT* cur = buf; cur < std::end(buf); cur += fixture.len) {
                sv.copy(cur, fixture.len);
            }

            LOCAL_EXPECT(bufvw.ends_with(sv));
            LOCAL_EXPECT(bufvw.ends_with(sv.data())); // assume NUL-termination
            LOCAL_EXPECT(not sv.ends_with(bufvw));    // sv shorter
            LOCAL_EXPECT(sv.ends_with(bufvw.back())); // single char
            LOCAL_EXPECT(not SV{}.ends_with(bufvw.back())); // SV{} empty

            for (size_t i = 0; i < std::size(buf); ++i) {
                LOCAL_EXPECT(bufvw.substr(i).ends_with(sv) ==
                             ((bufvw.size() - i) >= sv.length()));
            }
            for (size_t n = 0; n < std::size(buf); ++n) {
                LOCAL_EXPECT(bufvw.substr(0, n).ends_with(sv) ==
                             ((n > 0) && (0 == (n % sv.length()))));
            }
        }

        // find
        {
            SV const sv{fixture.sz1234};

            CharT buf[fixture.len*4] = {};
            SV bufvw{buf, std::size(buf)};

            for (CharT* cur = buf; cur < std::end(buf); cur += fixture.len) {
                sv.copy(cur, fixture.len);
            }

            LOCAL_EXPECT(bufvw.find(sv) == 0);
            LOCAL_EXPECT(sv.find(bufvw) == SV::npos); // needle larger than haystack
            LOCAL_EXPECT(bufvw.find(sv.data()) == 0); // assumes NUL termination
            LOCAL_EXPECT(bufvw.find(sv[2]) == 2);     // single CharT

            // constexpr size_type find( const CharT* s, size_type pos ) const;
            LOCAL_EXPECT(bufvw.find(sv.data() + 1, 4) == 5);
            // constexpr size_type find( const CharT* s, size_type pos, size_type count ) const;
            LOCAL_EXPECT(bufvw.substr(0,6).find(sv.data(), 2, 2) == 4);

            // empty haystack
            LOCAL_EXPECT(SV{}.find(sv[2]) == SV::npos);
            // empty needle
            LOCAL_EXPECT(bufvw.find({fixture.empty}) == 0);

            using Matches = std::vector<size_t>;
            auto matches = [](SV haystack, SV needle) {
                Matches res;
                for (size_t pos = 0; pos < haystack.size() &&
                     SV::npos != (pos = haystack.find(needle, pos));
                     ++pos) //
                {
                    res.push_back(pos);
                }
                return res;
            };

            LOCAL_EXPECT((matches(bufvw, sv) == Matches{0, 4, 8, 12}));
            LOCAL_EXPECT((matches(bufvw, sv.substr(1)) == Matches{1, 5, 9, 13}));
            LOCAL_EXPECT((matches(bufvw, sv.substr(2)) == Matches{2, 6, 10, 14}));
            LOCAL_EXPECT((matches(bufvw, sv.substr(3)) == Matches{3, 7, 11, 15}));
            LOCAL_EXPECT((matches(sv, bufvw) == Matches{}));
            LOCAL_EXPECT((matches(sv, {}) == Matches{0, 1, 2, 3}));

            bufvw.remove_suffix(1);
            LOCAL_EXPECT((matches(bufvw, sv) == Matches{0, 4, 8/*, now partial match: 12*/}));
        }

        // rfind
        {
            SV const sv{fixture.sz1234};

            CharT buf[fixture.len*4] = {};
            SV bufvw{buf, std::size(buf)};

            for (CharT* cur = buf; cur < std::end(buf); cur += fixture.len) {
                sv.copy(cur, fixture.len);
            }

            LOCAL_EXPECT(bufvw.rfind(sv) == 12);
            LOCAL_EXPECT(bufvw.rfind(sv, 12) == 12);
            LOCAL_EXPECT(sv.rfind(bufvw) == SV::npos); // needle larger than haystack
            LOCAL_EXPECT(bufvw.rfind(sv.data()) == 12); // assumes NUL termination
            LOCAL_EXPECT(bufvw.rfind(sv[2]) == 14);     // single CharT

            // constexpr size_type rfind( const CharT* s, size_type pos ) const;
            LOCAL_EXPECT(bufvw.rfind(sv.data() + 1, 12) == 9);
            //// constexpr size_type rfind( const CharT* s, size_type pos, size_type count ) const;
            LOCAL_EXPECT(bufvw.substr(0,6).rfind(sv.data(), 4, 2) == 4);

            // empty haystack
            LOCAL_EXPECT(SV{}.rfind(sv[2]) == SV::npos);
            // empty needle
            LOCAL_EXPECT(bufvw.rfind({fixture.empty}) == bufvw.size());

            using Matches = std::vector<size_t>;
            auto matches = [](SV haystack, SV needle) {
                Matches res;
                for (size_t pos = haystack.size();
                     SV::npos != (pos = haystack.rfind(needle, pos)); --pos) //
                {
                    res.push_back(pos);
                    if (pos == 0)
                        break;
                }
                return res;
            };

            LOCAL_EXPECT((matches(bufvw, sv) == Matches{12, 8 ,4, 0}));
            LOCAL_EXPECT((matches(bufvw, sv.substr(1)) == Matches{13, 9, 5, 1}));
            LOCAL_EXPECT((matches(bufvw, sv.substr(2)) == Matches{14, 10, 6, 2}));
            LOCAL_EXPECT((matches(bufvw, sv.substr(3)) == Matches{15, 11, 7, 3}));
            LOCAL_EXPECT((matches(sv, bufvw) == Matches{}));
            LOCAL_EXPECT((matches(sv, {}) == Matches{4, 3, 2, 1, 0})); // !!!

            bufvw.remove_prefix(1);
            LOCAL_EXPECT((matches(bufvw, sv) == Matches{11,7,3}));
        }

        // contains
        if constexpr (!skip_contains) {
            SV const sv{fixture.sz1234};

            CharT buf[fixture.len*4] = {};
            SV bufvw{buf, std::size(buf)};

            for (CharT* cur = buf; cur < std::end(buf); cur += fixture.len) {
                sv.copy(cur, fixture.len);
            }

            LOCAL_EXPECT(bufvw.contains(sv));
            LOCAL_EXPECT(not sv.contains(bufvw));
            LOCAL_EXPECT(sv.contains(SV{})); // anything contains empty strings
            LOCAL_EXPECT(SV{}.contains(SV{})); // empty strings contain empty strings
        }
    }

    void check_relational() { // TODO SEHE
    }

    void check_streaming() { // TODO SEHE
    }

    void check_hashing() {
        std_hashing();
        boost_hashing();
    }

    void check_argument_passing() {
        // Function call arguments and implicit uniform construction
        LOCAL_EXPECT(acceptSV(SV{}));
        LOCAL_EXPECT(acceptSV({}));
        LOCAL_EXPECT(acceptSV({fixture.sz1234}));
        LOCAL_EXPECT(acceptSV({fixture.sz1234, fixture.len}));
    }

    void check_constructors() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
        //#if __cpp_lib_constexpr_string_view >= 201907
        // https://stackoverflow.com/a/53638114/85371
        [[maybe_unused]] [[clang::
                               require_constant_initialization]] static constexpr SV
            s_instance{fixture.sz1234, fixture.len};
#pragma GCC diagnostic pop
        //#endif

        // Default constructed is detectable as nullptr
        LOCAL_EXPECT(SV{}.data() == nullptr);

        // rest separated into check_non_empty_instances/check_empty_instances
    }

    void check_copy_and_assign() {
        SV const instance{fixture.sz1234};

        SV const copy { instance }; // copy construct
        LOCAL_EXPECT(copy == instance);
        LOCAL_EXPECT(copy.data() == instance.data());
        LOCAL_EXPECT(copy.length() == instance.length());
        LOCAL_EXPECT(not copy.empty());

        {
            auto mut_copy = copy; // copy-init
            LOCAL_EXPECT(copy == instance);
            LOCAL_EXPECT(copy.data() == instance.data());
            LOCAL_EXPECT(copy.length() == instance.length());
            LOCAL_EXPECT(not copy.empty());

            mut_copy = fixture.empty;
            LOCAL_EXPECT(mut_copy == fixture.empty);
            LOCAL_EXPECT(mut_copy.data() == fixture.empty);
            LOCAL_EXPECT(mut_copy.length() == 0);
            LOCAL_EXPECT(mut_copy.empty());
            
            mut_copy = {};
            LOCAL_EXPECT(mut_copy == fixture.empty);
            LOCAL_EXPECT(mut_copy.data() == nullptr);
            LOCAL_EXPECT(mut_copy.length() == 0);
            LOCAL_EXPECT(mut_copy.empty());

            mut_copy = {fixture.sz1234, fixture.len};
            LOCAL_EXPECT(copy == instance);
            LOCAL_EXPECT(copy.data() == instance.data());
            LOCAL_EXPECT(copy.length() == instance.length());
            LOCAL_EXPECT(not copy.empty());
        }
    }

    void check_non_empty_instances() {
        // Non-empty instance construction
        std::vector<SV> svs{
                 SV{fixture.sz1234},
                 SV{fixture.sz1234, fixture.len},
                 SV{fixture.sz1234, static_cast<unsigned>(fixture.len)}, // widening ok
             };

        // construct from iterator pair
#if __cpp_lib_string_view >= 201811 // TODO FIXME wrong feature test - but can't
                                    // find better for the C++23 feature
        // this will be OBSERVABLE if in c++23 mode, no doubt
        svs.emplace_back(fixture.sz1234, fixture.sz1234 + fixture.len);
#endif

        // TODO c++23 range constructor?

        for (SV const& sv : svs) {
            LOCAL_EXPECT(sv.length() == fixture.len);
            LOCAL_EXPECT(sv.size() == fixture.len);
            LOCAL_EXPECT(sv.data() == +fixture.sz1234);
            LOCAL_EXPECT(sv.begin() == +fixture.sz1234);
            LOCAL_EXPECT(sv.cbegin() == +fixture.sz1234);
            LOCAL_EXPECT(sv.end() == fixture.sz1234 + fixture.len);
            LOCAL_EXPECT(sv.cend() == fixture.sz1234 + fixture.len);
            LOCAL_EXPECT(not sv.empty());

            // reverse
            LOCAL_EXPECT(sv.crbegin() == sv.rbegin());
            LOCAL_EXPECT(sv.crend() == sv.rend());

            LOCAL_EXPECT(*sv.rbegin() == fixture.sz1234[fixture.len - 1]);
            LOCAL_EXPECT(sv.rend().base() == sv.begin());
            LOCAL_EXPECT(std::distance(sv.begin(), sv.end()) ==
                         static_cast<difference_type>(sv.size()));
            LOCAL_EXPECT(std::distance(sv.rbegin(), sv.rend()) ==
                         static_cast<difference_type>(sv.size()));

            {
                using std::begin;
                using std::end;
                using std::cbegin;
                using std::cend;
                using std::size;
                using std::empty;
                LOCAL_EXPECT(begin(sv) == +fixture.sz1234);
                LOCAL_EXPECT(end(sv) == fixture.sz1234 + fixture.len);
                LOCAL_EXPECT(cbegin(sv) == +fixture.sz1234);
                LOCAL_EXPECT(cend(sv) == fixture.sz1234 + fixture.len);
                LOCAL_EXPECT(size(sv) == fixture.len);
                LOCAL_EXPECT(not empty(sv));
            }

// for MSVC see https://stackoverflow.com/a/58316194/85371
#if __cplusplus >= 201402L
            {
                using std::rbegin;
                using std::rend;
                using std::crbegin;
                using std::crend;
                LOCAL_EXPECT(rbegin(sv)  == sv.rbegin());
                LOCAL_EXPECT(rend(sv)    == sv.rend());
                LOCAL_EXPECT(crbegin(sv) == sv.rbegin());
                LOCAL_EXPECT(crend(sv)   == sv.rend());
            }
#endif
        }
    }

    void check_empty_instances() {
        for (SV const& sv : {
                 SV{fixture.empty},
                 SV{fixture.sz1234, 0},
                 SV{},
             }) {
            LOCAL_EXPECT(sv.length() == 0);
            LOCAL_EXPECT(sv.size() == 0);
            LOCAL_EXPECT(sv.end() == sv.begin());
            LOCAL_EXPECT(sv.cend() == sv.cbegin());
            LOCAL_EXPECT(sv.empty());

            {
                using std::begin;
                using std::end;
                using std::cbegin;
                using std::cend;
                using std::size;
                using std::empty;
                LOCAL_EXPECT(end(sv) == begin(sv));
                LOCAL_EXPECT(cend(sv) == cbegin(sv));
                LOCAL_EXPECT(size(sv) == 0);
                LOCAL_EXPECT(empty(sv));
            }
        }
    }

private: // utils
    static bool acceptSV(SV sv) {
        return true;
    }

    template <typename T> static bool acceptSV(T const& sv, ...) {
        return false;
    }

    template <typename Hash> static void exercise_hash(Hash const& hash) {
        size_t h1 = hash(SV{});
        size_t h2 = hash(SV{fixture.empty});
        size_t h3 = hash(SV{fixture.sz1234});
        size_t h4 = hash(SV{fixture.sz1234,fixture.len});
        LOCAL_EXPECT(h1 == h2);
        LOCAL_EXPECT(h1 != h3);
        LOCAL_EXPECT(h2 != h4);
        LOCAL_EXPECT(h3 == h4);
    }

    template <typename H = std::hash<SV>> static void std_hashing(H* = {}) {
        exercise_hash(H{});
    }
    static void std_hashing(...) {
        //BEAST_EXPECTS(false, __PRETTY_FUNCTION__);// TODO FIXME useful feedback
    }

    template <typename H = boost::hash<SV>> static void boost_hashing(H* = {}) {
        exercise_hash(H{});
    }
    static void boost_hashing(...) {
        //BEAST_EXPECTS(false, __PRETTY_FUNCTION__);// TODO FIXME useful feedback
    }
#undef LOCAL_EXPECT
};

template <template <typename...> class StringViewTemplate>
struct CheckTemplate {
    void run() {
        CheckInstance<StringViewTemplate<char>,     char>     _string_view;
        CheckInstance<StringViewTemplate<wchar_t>,  wchar_t>  _wstring_view;
        CheckInstance<StringViewTemplate<char16_t>, char16_t> _u16string_view;
        CheckInstance<StringViewTemplate<char32_t>, char32_t> _u32string_view;

        _string_view.run();
        _wstring_view.run();
        _u16string_view.run();
        _u32string_view.run();

#if __cpp_char8_t >= 201907
        CheckInstance<StringViewTemplate<char8_t>,  char8_t>    _u8string_view;
        _u8string_view.run();
#endif
    }
};

} // namespace string_views
} // namespace test_detail

class string_test : public unit_test::suite {
public:
    void run() override {
        using namespace test_detail::string_views;

#if BEAST_TEST_HAVE_STD_STRING_VIEW
        CheckTemplate<std::basic_string_view>{}.run();
#elif defined(__cpp_lib_experimental_string_view)
        CheckTemplate<std::experimental::basic_string_view>{}.run();
#endif
        CheckTemplate<boost::basic_string_view>{}.run();
        CheckTemplate<boost::core::basic_string_view>{}.run();
    }
};

BEAST_DEFINE_TESTSUITE(beast, core, string);

} // beast
} // boost
