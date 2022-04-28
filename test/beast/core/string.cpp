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
#include <iostream> // TODO FIXME remove
#if __has_include(<version>)
    #include <version> // for feature test macros to be reliable
#else
    #pragma message "no reliable feature test macros"
#endif

#ifdef __cpp_lib_string_view
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
    static constexpr CharT const four[] = "1234";
    static constexpr size_t len = std::size(four) - 1; // not including NUL
};

template <> struct Fixture<wchar_t> {
    static constexpr wchar_t const empty[] = L"";
    static constexpr wchar_t const four[] = L"1234";
    static constexpr size_t len = std::size(four) - 1; // not including NUL
};

template <> struct Fixture<char16_t> {
    static constexpr char16_t const empty[] = u"";
    static constexpr char16_t const four[] = u"1234";
    static constexpr size_t len = std::size(four) - 1; // not including NUL
};

template <> struct Fixture<char32_t> {
    static constexpr char32_t const empty[] = U"";
    static constexpr char32_t const four[] = U"1234";
    static constexpr size_t len = std::size(four) - 1; // not including NUL
};

template <typename SV, typename CharT> struct CheckInstance {
    static constexpr Fixture<CharT> fixture{};
    static_assert(fixture.len > 1);

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
            SV const instance{fixture.four};

            for (size_t i = 0; i < instance.length(); ++i)
                LOCAL_EXPECT(instance[i] == fixture.four[i]);

            for (size_t i = 0; i < instance.length(); ++i)
                LOCAL_EXPECT(instance.at(i) == fixture.four[i]);

            LOCAL_EXPECT(instance.front() == instance[0]);
            LOCAL_EXPECT(std::addressof(instance.front()) == std::addressof(instance[0]));
            LOCAL_EXPECT(std::addressof(instance.front()) == instance.data());

            LOCAL_EXPECT(instance.back() == instance[instance.length() - 1]);
            LOCAL_EXPECT(std::addressof(instance.back()) == std::addressof(instance[instance.length() - 1]));
        }

        { 
            SV const instance{fixture.four, fixture.len - 1};

#ifndef NDEBUG
            // skipped because implementations may assert
#else
            instance[instance.length()]; // doesn't throw, also no UB due
                                         // underlying fixture::four
#endif

            BEAST_THROWS(instance.at(instance.length()), std::out_of_range);
        }
    }

    void check_capacity() {
        check_capacity_max_size(); // this is its own saga...
    }

    void check_capacity_max_size() {
        static_assert(
            std::is_same_v<decltype(std::declval<SV>().max_size()), size_t>);

        // OBSERVABLE differences:
        if constexpr (is_instance_v<SV, boost::basic_string_view>) {
            // boost::basic_string_view erroneously returns max_size() as
            // length()
            LOCAL_EXPECT(0 == SV{}.max_size()); // oops
#ifdef __cpp_lib_string_view
        } else if constexpr (is_instance_v<SV, std::basic_string_view>) {
            static_assert(SV{}.max_size() ==
                          (std::numeric_limits<difference_type>::max() - 8) /
                              (2 * sizeof(CharT)));
#endif
        } else {
            // size/length see check_non_empty_instances/check_empty_instances
            static_assert(SV{}.max_size() ==
                          (std::numeric_limits<size_t>::max()) /
                              (sizeof(CharT)));
        }
    }

    void check_modifiers() { // TODO SEHE
    }

    void check_operations() { // TODO SEHE
#if __cpp_lib_starts_ends_with >= 201711
#endif
#if __cpp_lib_string_contains >= 202011
#endif
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
        LOCAL_EXPECT(acceptSV({fixture.four}));
        LOCAL_EXPECT(acceptSV({fixture.four, fixture.len}));
    }

    void check_constructors() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
        //#if __cpp_lib_constexpr_string_view >= 201907
        // https://stackoverflow.com/a/53638114/85371
        [[maybe_unused]] [[clang::
                               require_constant_initialization]] static constexpr SV
            s_instance{fixture.four, fixture.len};
#pragma GCC diagnostic pop
        //#endif

        // Default constructed is detectable as nullptr
        LOCAL_EXPECT(SV{}.data() == nullptr);

        // rest separated into check_non_empty_instances/check_empty_instances
    }

    void check_copy_and_assign() {
        SV const instance{fixture.four};

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

            mut_copy = {fixture.four, fixture.len};
            LOCAL_EXPECT(copy == instance);
            LOCAL_EXPECT(copy.data() == instance.data());
            LOCAL_EXPECT(copy.length() == instance.length());
            LOCAL_EXPECT(not copy.empty());
        }
    }

    void check_non_empty_instances() {
        // Non-empty instance construction
        std::vector<SV> svs{
                 SV{fixture.four},
                 SV{fixture.four, fixture.len},
                 SV{fixture.four, static_cast<unsigned>(fixture.len)}, // widening ok
             };

        // construct from iterator pair
#if __cpp_lib_string_view >= 201811 // TODO FIXME wrong feature test - but can't
                                    // find better for the C++23 feature
        // this will be OBSERVABLE if in c++23 mode, no doubt
        svs.emplace_back(fixture.four, fixture.four + fixture.len);
#endif

        // TODO c++23 range constructor?

        for (SV const& sv : svs) {
            LOCAL_EXPECT(sv.length() == fixture.len);
            LOCAL_EXPECT(sv.size() == fixture.len);
            LOCAL_EXPECT(sv.data() == +fixture.four);
            LOCAL_EXPECT(sv.begin() == +fixture.four);
            LOCAL_EXPECT(sv.cbegin() == +fixture.four);
            LOCAL_EXPECT(sv.end() == fixture.four + fixture.len);
            LOCAL_EXPECT(sv.cend() == fixture.four + fixture.len);
            LOCAL_EXPECT(not sv.empty());

            // reverse
            LOCAL_EXPECT(sv.crbegin() == sv.rbegin());
            LOCAL_EXPECT(sv.crend() == sv.rend());

            LOCAL_EXPECT(*sv.rbegin() == fixture.four[fixture.len - 1]);
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
                LOCAL_EXPECT(begin(sv) == +fixture.four);
                LOCAL_EXPECT(end(sv) == fixture.four + fixture.len);
                LOCAL_EXPECT(cbegin(sv) == +fixture.four);
                LOCAL_EXPECT(cend(sv) == fixture.four + fixture.len);
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
                 SV{fixture.four, 0},
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
        size_t h3 = hash(SV{fixture.four});
        size_t h4 = hash(SV{fixture.four,fixture.len});
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

#ifdef __cpp_lib_string_view
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
