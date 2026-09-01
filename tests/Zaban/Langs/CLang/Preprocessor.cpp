#include <gtest/gtest.h>

#include <Z/Zaban/Langs/CLang/Lexer.hpp>
#include <Z/Zaban/Langs/CLang/Preprocessor.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace Z::Zaban::Tests {
    using namespace Z::Zaban::Langs::CLang;

    namespace {
        /// Lexes then preprocesses, the way the driver will.
        std::vector<CLexerTokenType> pp_tokens(std::string_view src) {
            CLexerBufferType buffer = src;
            CLexer           lexer(buffer);
            lexer.scan();

            CPreprocessor pp(buffer);
            return pp.process(lexer.finalize());
        }

        bool token_has(const CLexerTokenType& t, TokenFlags f) {
            return has(static_cast<TokenFlags>(t.flags), f);
        }

        std::size_t count_flagged(const std::vector<CLexerTokenType>& tokens,
                                  TokenFlags                          f) {
            std::size_t n = 0;
            for (const auto& t: tokens) {
                if (token_has(t, f)) ++n;
            }
            return n;
        }

        std::string describe(const std::vector<CLexerTokenType>& tokens) {
            std::string out;
            for (std::size_t i = 0; i < tokens.size(); ++i) {
                if (i > 0) out += ", ";
                out += to_string(tokens[i].kind);
                if (token_has(tokens[i], TokenFlags::DirectiveLine)) {
                    out += "*";
                }
            }
            return out;
        }
        /// Renders spelling, so expansion results are readable in failures.
        std::string text_of(std::string_view                    src,
                            const std::vector<CLexerTokenType>& tokens) {
            std::string out;
            for (const auto& t: tokens) {
                if (t.kind == CLexerTokenKind::Eob) continue;
                if (token_has(t, TokenFlags::DirectiveLine)) continue;
                if (!out.empty()) out += " ";
                out += std::string(
                    src.substr(t.range.begin, t.range.end - t.range.begin));
            }
            return out;
        }

        /// Kinds of the non-directive tokens, which is what a compiler sees.
        std::vector<CLexerTokenKind> code_kinds(
            const std::vector<CLexerTokenType>& tokens) {
            std::vector<CLexerTokenKind> out;
            for (const auto& t: tokens) {
                if (!token_has(t, TokenFlags::DirectiveLine)) {
                    out.push_back(t.kind);
                }
            }
            return out;
        }
    }  // namespace
    /**
     * Expect: WhiteSpaceBefore distinguishes function-like from object-like.
     * Should: `F(x)` has no space before `(`; `F (x)` does. This is the only
     * thing that separates the two forms, and it is unrecoverable after
     * lexing.
     */
    TEST(CPreprocessorTest, ObjectLikeVsFunctionLike) {
        const std::vector<CLexerTokenType> fn  = pp_tokens("#define F(x) x");
        const std::vector<CLexerTokenType> obj = pp_tokens("#define F (x) x");

        ASSERT_GE(fn.size(), 4u) << describe(fn);
        ASSERT_GE(obj.size(), 4u) << describe(obj);

        ASSERT_EQ(fn[3].kind, CLexerTokenKind::LParen);
        ASSERT_EQ(obj[3].kind, CLexerTokenKind::LParen);

        EXPECT_FALSE(token_has(fn[3], TokenFlags::WhiteSpaceBefore))
            << "function-like";
        EXPECT_TRUE(token_has(obj[3], TokenFlags::WhiteSpaceBefore))
            << "object-like";
    }
    /**
     * Expect: every token on the directive line is marked, and nothing else.
     * Should: `#define F 1` marks 4 tokens; the trailing `int x;` is untouched.
     */
    TEST(CPreprocessorTest, MarksDirectiveLineOnly) {
        const std::vector<CLexerTokenType> t = pp_tokens("#define F 1\nint x;");

        EXPECT_EQ(count_flagged(t, TokenFlags::DirectiveLine), 4u)
            << describe(t);
        EXPECT_TRUE(token_has(t[0], TokenFlags::DirectiveLine)) << "#";
        EXPECT_TRUE(token_has(t[3], TokenFlags::DirectiveLine)) << "1";
        EXPECT_FALSE(token_has(t[4], TokenFlags::DirectiveLine)) << "int";
    }

    /**
     * Expect: a Hash not at line start is not a directive.
     * Should: the `#` in a macro body is left alone.
     */
    TEST(CPreprocessorTest, MidLineHashIsNotADirective) {
        const std::vector<CLexerTokenType> t = pp_tokens("int x; # y");

        EXPECT_EQ(count_flagged(t, TokenFlags::DirectiveLine), 0u)
            << describe(t);
    }

    /**
     * Expect: a directive may be indented.
     * Should: leading whitespace does not clear AtLineStart.
     */
    TEST(CPreprocessorTest, IndentedDirective) {
        const std::vector<CLexerTokenType> t = pp_tokens("  #define F 1");

        EXPECT_EQ(count_flagged(t, TokenFlags::DirectiveLine), 4u)
            << describe(t);
    }

    /**
     * Expect: a bare `#` on its own line is a null directive, still marked.
     */
    TEST(CPreprocessorTest, NullDirective) {
        const std::vector<CLexerTokenType> t = pp_tokens("#\nint x;");

        EXPECT_TRUE(token_has(t[0], TokenFlags::DirectiveLine));
        EXPECT_FALSE(token_has(t[1], TokenFlags::DirectiveLine)) << describe(t);
    }

    /**
     * Expect: two directives on consecutive lines are two separate lines.
     */
    TEST(CPreprocessorTest, ConsecutiveDirectives) {
        const std::vector<CLexerTokenType> t =
            pp_tokens("#define A 1\n#define B 2\n");

        EXPECT_EQ(count_flagged(t, TokenFlags::DirectiveLine), 8u)
            << describe(t);
    }
    /**
     * Expect: `%:` is a directive opener exactly like `#`.
     * Should: the digraph merges to Hash and keeps AtLineStart.
     */
    TEST(CPreprocessorTest, DigraphDirective) {
        const std::vector<CLexerTokenType> t = pp_tokens("%:define F 1");

        ASSERT_GE(t.size(), 1u);
        EXPECT_EQ(t[0].kind, CLexerTokenKind::Hash);
        EXPECT_EQ(count_flagged(t, TokenFlags::DirectiveLine), 4u)
            << describe(t);
    }

    /**
     * Expect: a directive line continued by a splice is one logical line.
     * Should: the tokens after the splice stay on the directive line, because
     * splicing removed the newline before AtLineStart could be set.
     */
    TEST(CPreprocessorTest, SplicedDirectiveContinuation) {
        const std::vector<CLexerTokenType> t =
            pp_tokens("#define F \\\n1\nint x;");

        EXPECT_EQ(count_flagged(t, TokenFlags::DirectiveLine), 4u)
            << describe(t);
        EXPECT_FALSE(token_has(t[4], TokenFlags::DirectiveLine)) << "int";
    }

    /**
     * Expect: the cursed torture case is exactly one directive line.
     */
    TEST(CPreprocessorTest, CursedDirective) {
        static constexpr std::string_view src =
            "/\\\n"
            "*\n"
            "*/ # /*\n"
            "*/ defi\\\n"
            "ne FO\\\n"
            "O 10\\\n"
            "20\n";

        const std::vector<CLexerTokenType> t = pp_tokens(src);

        EXPECT_EQ(count_flagged(t, TokenFlags::DirectiveLine), 4u)
            << describe(t);
    }
    /**
     * Expect: process() currently changes nothing but flags.
     * Should: same token count and kinds in, same out.
     * TODO: this fails right now but should fix it later on
     * if we aim to use the lexer/pp for LSP as well as the compiler, this will
     * become an issue
     */
    // TEST(CPreprocessorTest, PreservesStream) {
    //     CLexerBufferType buffer = "#define F 1\nint x = F;";
    //     CLexer           lexer(buffer);
    //     lexer.scan();
    //     const std::vector<CLexerTokenType> before = lexer.finalize();

    //     CPreprocessor                      pp(buffer);
    //     const std::vector<CLexerTokenType> after = pp.process(before);

    //     ASSERT_EQ(after.size(), before.size());
    //     for (std::size_t i = 0; i < before.size(); ++i) {
    //         EXPECT_EQ(after[i].kind, before[i].kind) << "index " << i;
    //         EXPECT_EQ(after[i].range.begin, before[i].range.begin);
    //     }
    // }

    /**
     * Expect: an object-like macro replaces its name at the use site.
     */
    TEST(CPreprocessorTest, ObjectLikeSubstitution) {
        static constexpr std::string_view src = "#define ONE 1\nint x = ONE;";
        const auto                        t   = pp_tokens(src);

        EXPECT_EQ(text_of(src, t), "int x = 1 ;");
    }

    /**
     * Expect: a macro is invisible before its #define.
     */
    TEST(CPreprocessorTest, NotVisibleBeforeDefinition) {
        static constexpr std::string_view src = "int x = F;\n#define F 1";
        const auto                        t   = pp_tokens(src);

        EXPECT_EQ(text_of(src, t), "int x = F ;");
    }

    /**
     * Expect: #undef removes it; later uses are plain identifiers.
     */
    TEST(CPreprocessorTest, Undef) {
        static constexpr std::string_view src =
            "#define F 1\nint a = F;\n#undef F\nint b = F;";
        const auto t = pp_tokens(src);

        EXPECT_EQ(text_of(src, t), "int a = 1 ; int b = F ;");
    }

    /**
     * Expect: a body of several tokens expands to all of them.
     */
    TEST(CPreprocessorTest, MultiTokenBody) {
        static constexpr std::string_view src = "#define P 1 + 2\nint x = P;";
        const auto                        t   = pp_tokens(src);

        EXPECT_EQ(text_of(src, t), "int x = 1 + 2 ;");
    }

    /**
     * Expect: an empty body expands to nothing.
     */
    TEST(CPreprocessorTest, EmptyMacro) {
        static constexpr std::string_view src =
            "#define EMPTY\nint e = 0 EMPTY;";
        const auto t = pp_tokens(src);

        EXPECT_EQ(text_of(src, t), "int e = 0 ;");
    }
    /**
     * Expect: a replacement is rescanned, so nested names expand too.
     */
    TEST(CPreprocessorTest, RescansReplacement) {
        static constexpr std::string_view src =
            "#define ONE 1\n#define TWO (ONE + ONE)\nint x = TWO;";
        const auto t = pp_tokens(src);

        EXPECT_EQ(text_of(src, t), "int x = ( 1 + 1 ) ;");
    }

    /**
     * Expect: order of definition does not matter, only order of use.
     * TWO is defined before ONE, but ONE exists by the time TWO expands.
     */
    TEST(CPreprocessorTest, ForwardReferenceInBody) {
        static constexpr std::string_view src =
            "#define TWO (ONE + ONE)\n#define ONE 1\nint x = TWO;";
        const auto t = pp_tokens(src);

        EXPECT_EQ(text_of(src, t), "int x = ( 1 + 1 ) ;");
    }
    /**
     * Expect: a self-referential macro expands exactly once.
     */
    TEST(CPreprocessorTest, DirectRecursionExpandsOnce) {
        static constexpr std::string_view src =
            "#define recur_var (1 + recur_var)\nint x = recur_var;";
        const auto t = pp_tokens(src);

        EXPECT_EQ(text_of(src, t), "int x = ( 1 + recur_var ) ;");
    }

    /**
     * Expect: a two-macro cycle terminates.
     */
    TEST(CPreprocessorTest, MutualRecursionTerminates) {
        static constexpr std::string_view src =
            "#define p_var q_var\n#define q_var p_var\nint x = p_var;";
        const auto t = pp_tokens(src);

        EXPECT_EQ(text_of(src, t), "int x = p_var ;");
    }
    /**
     * Expect: the first replacement token takes the invocation's position.
     * Body tokens carry the #define line's spacing, not the call site's.
     */
    TEST(CPreprocessorTest, ReplacementInheritsLineStart) {
        const auto t = pp_tokens("#define H 1\nH;");

        const auto code = code_kinds(t);
        ASSERT_GE(code.size(), 1u);

        for (const auto& tok: t) {
            if (token_has(tok, TokenFlags::DirectiveLine)) continue;
            EXPECT_TRUE(token_has(tok, TokenFlags::AtLineStart));
            break;
        }
    }

    /**
     * Expect: a macro that expands to # at line start is still a directive
     * opener as far as the flags are concerned.
     */
    TEST(CPreprocessorTest, ReplacementKeepsWhitespaceBefore) {
        const auto t = pp_tokens("#define V 1\nint x =V;");

        for (const auto& tok: t) {
            if (token_has(tok, TokenFlags::DirectiveLine)) continue;
            if (tok.kind != CLexerTokenKind::Numeric) continue;
            EXPECT_FALSE(token_has(tok, TokenFlags::WhiteSpaceBefore));
            break;
        }
    }
    /**
     * Expect: names on directive lines are not expanded.
     */
    TEST(CPreprocessorTest, DirectiveLineNotExpanded) {
        static constexpr std::string_view src =
            "#define A 1\n#define B A\nint x = B;";
        const auto t = pp_tokens(src);

        // B's body still holds the token A; it expands at B's use site.
        EXPECT_EQ(text_of(src, t), "int x = 1 ;");
    }

    /**
     * Expect: a function-like macro substitutes its arguments.
     */
    TEST(CPreprocessorTest, FunctionLikeSubstitution) {
        static constexpr std::string_view src =
            "#define ADD(a, b) ((a) + (b))\nint x = ADD(1, 2);";
        const auto t = pp_tokens(src);

        EXPECT_EQ(text_of(src, t), "int x = ( ( 1 ) + ( 2 ) ) ;");
    }

    /**
     * Expect: commas inside parens belong to the argument, not the list.
     */
    TEST(CPreprocessorTest, NestedParensInArgument) {
        static constexpr std::string_view src =
            "#define ADD(a, b) ((a) + (b))\n"
            "#define MAX(a, b) ((a) > (b) ? (a) : (b))\n"
            "int x = MAX(ADD(1, 2), 2);";
        const auto t = pp_tokens(src);

        EXPECT_EQ(text_of(src, t),
                  "int x = ( ( ( ( 1 ) + ( 2 ) ) ) > ( 2 ) ? "
                  "( ( ( 1 ) + ( 2 ) ) ) : ( 2 ) ) ;");
    }

    /**
     * Expect: an invocation may span lines between the name and the `(`.
     */
    TEST(CPreprocessorTest, InvocationSpansLines) {
        static constexpr std::string_view src =
            "#define ID(x) x\nint m = ID\n(7);";
        const auto t = pp_tokens(src);

        EXPECT_EQ(text_of(src, t), "int m = 7 ;");
    }

    /**
     * Expect: a function-like name without `(` is a plain identifier.
     */
    TEST(CPreprocessorTest, NameWithoutParenIsNotInvocation) {
        static constexpr std::string_view src =
            "#define ADD(a, b) ((a) + (b))\nint x = ADD;";
        const auto t = pp_tokens(src);

        EXPECT_EQ(text_of(src, t), "int x = ADD ;");
    }

    /**
     * Expect: zero-parameter macros take an empty argument list.
     */
    TEST(CPreprocessorTest, ZeroParamInvocation) {
        static constexpr std::string_view src = "#define N() 5\nint x = N();";
        const auto                        t   = pp_tokens(src);

        EXPECT_EQ(text_of(src, t), "int x = 5 ;");
    }

    /**
     * Expect: an empty argument substitutes nothing.
     */
    TEST(CPreprocessorTest, EmptyArgument) {
        static constexpr std::string_view src =
            "#define ID(x) [x]\nint a = ID();";
        const auto t = pp_tokens(src);

        EXPECT_EQ(text_of(src, t), "int a = [ ] ;");
    }

    /**
     * Expect: wrong arity leaves the name unexpanded rather than substituting.
     */
    TEST(CPreprocessorTest, ArityMismatchDoesNotExpand) {
        static constexpr std::string_view src =
            "#define ADD(a, b) ((a) + (b))\nint x = ADD(1);";
        const auto t = pp_tokens(src);

        EXPECT_EQ(text_of(src, t), "int x = ADD ( 1 ) ;");
    }

    /**
     * Expect: a parameter used twice substitutes twice.
     */
    TEST(CPreprocessorTest, RepeatedParameter) {
        static constexpr std::string_view src =
            "#define SQ(x) ((x) * (x))\nint x = SQ(3);";
        const auto t = pp_tokens(src);

        EXPECT_EQ(text_of(src, t), "int x = ( ( 3 ) * ( 3 ) ) ;");
    }
    /**
     * Expect: arguments are expanded before substitution, in the caller's
     * context where the macro is not yet hidden. This is the case an
     * active-name set cannot express.
     */
    TEST(CPreprocessorTest, ArgumentPreExpansion) {
        static constexpr std::string_view src =
            "#define f(x) x + f(x)\nint r = f(f(1));";
        const auto t = pp_tokens(src);

        EXPECT_EQ(text_of(src, t), "int r = 1 + f ( 1 ) + f ( 1 + f ( 1 ) ) ;");
    }

    /**
     * Expect: an argument that is itself a macro expands once.
     */
    TEST(CPreprocessorTest, MacroAsArgument) {
        static constexpr std::string_view src =
            "#define ONE 1\n#define ID(x) x\nint r = ID(ONE);";
        const auto t = pp_tokens(src);

        EXPECT_EQ(text_of(src, t), "int r = 1 ;");
    }

    /**
     * Expect: indirect self-reference through an argument still terminates.
     */
    TEST(CPreprocessorTest, ArgumentCarriesHideSet) {
        static constexpr std::string_view src =
            "#define g(x) x\n#define h(x) g(x)\nint r = h(h(1));";
        const auto t = pp_tokens(src);

        EXPECT_EQ(text_of(src, t), "int r = 1 ;");
    }

    /**
     * Expect: a function-like macro that recurses terminates.
     */
    TEST(CPreprocessorTest, FunctionLikeRecursion) {
        static constexpr std::string_view src =
            "#define r(x) r(x)\nint a = r(1);";
        const auto t = pp_tokens(src);

        EXPECT_EQ(text_of(src, t), "int a = r ( 1 ) ;");
    }
    TEST(CPreprocessorTest, IfTakesTrueBranch) {
        static constexpr std::string_view src =
            "#if 1\n#define A 1\n#else\n#define A 2\n#endif\nint x = A;";
        EXPECT_EQ(text_of(src, pp_tokens(src)), "int x = 1 ;");
    }

    TEST(CPreprocessorTest, ElifAfterTakenBranchIsSkipped) {
        static constexpr std::string_view src =
            "#if 1\n#define A 1\n#elif 1\n#define A 2\n#endif\nint x = A;";
        EXPECT_EQ(text_of(src, pp_tokens(src)), "int x = 1 ;");
    }

    TEST(CPreprocessorTest, ElifChainPicksFirstTrue) {
        static constexpr std::string_view src =
            "#if 0\n#define A 1\n#elif 2 > 1\n#define A 2\n"
            "#elif 1\n#define A 3\n#else\n#define A 4\n#endif\nint x = A;";
        EXPECT_EQ(text_of(src, pp_tokens(src)), "int x = 2 ;");
    }

    TEST(CPreprocessorTest, IfdefAndIfndef) {
        static constexpr std::string_view src =
            "#define P\n#ifdef P\n#define A 1\n#endif\n"
            "#ifndef Q\n#define B 2\n#endif\nint x = A + B;";
        EXPECT_EQ(text_of(src, pp_tokens(src)), "int x = 1 + 2 ;");
    }

    TEST(CPreprocessorTest, UndefClosesIfdef) {
        static constexpr std::string_view src =
            "#define T 1\n#undef T\n#ifdef T\n#define A 1\n#endif\n"
            "#ifndef T\n#define A 2\n#endif\nint x = A;";
        EXPECT_EQ(text_of(src, pp_tokens(src)), "int x = 2 ;");
    }

    /**
     * Expect: a skipped group is never evaluated, so it may hold anything.
     * Nesting is still tracked so the right #endif matches.
     */
    TEST(CPreprocessorTest, SkippedGroupIsNeverLookedAt) {
        static constexpr std::string_view src =
            "#if 0\n@ $ +++ ]]]\n#if 1\n#define BAD 1\n#endif\n"
            "#define ALSO_BAD 1\n#endif\n"
            "#ifdef BAD\n#define A 1\n#endif\n"
            "#ifndef ALSO_BAD\n#define A 2\n#endif\nint x = A;";
        EXPECT_EQ(text_of(src, pp_tokens(src)), "int x = 2 ;");
    }

    TEST(CPreprocessorTest, SkippedTokensAreMarkedNotDropped) {
        static constexpr std::string_view src = "#if 0\nint dead;\n#endif\n";
        const auto                        t   = pp_tokens(src);

        const auto it =
            std::find_if(t.begin(), t.end(), [](const CLexerTokenType& x) {
                return CLexerTokenKind::Int == x.kind;
            });
        ASSERT_NE(it, t.end());
        EXPECT_TRUE(token_has(*it, TokenFlags::Skipped));
    }

    /**
     * Expect: defined() is resolved before expansion, and a name that was
     * never defined evaluates to 0.
     */
    TEST(CPreprocessorTest, DefinedOperator) {
        static constexpr std::string_view src =
            "#define P 1\n"
            "#if defined(P) && defined Q == 0 && NEVER == 0\n"
            "#define A 1\n#endif\nint x = A;";
        EXPECT_EQ(text_of(src, pp_tokens(src)), "int x = 1 ;");
    }

    TEST(CPreprocessorTest, ConditionIsMacroExpanded) {
        static constexpr std::string_view src =
            "#define V 2\n#define MAX(a,b) ((a) > (b) ? (a) : (b))\n"
            "#if MAX(V, 1) == 2\n#define A 1\n#endif\nint x = A;";
        EXPECT_EQ(text_of(src, pp_tokens(src)), "int x = 1 ;");
    }

    /**
     * Expect: char constants, integer division
     * and remainder.
     */
    TEST(CPreprocessorTest, ConditionArithmetic) {
        static constexpr std::string_view src =
            "#if 'A' == 65 && (1 / 2) == 0 && (3 % 2) == 1\n"
            "#define A 1\n#else\n#define A 0\n#endif\nint x = A;";
        EXPECT_EQ(text_of(src, pp_tokens(src)), "int x = 1 ;");
    }

    /**
     * Expect: the arithmetic runs in the widest
     * integer type and unsigned wins, so -1 is a huge value and the
     * comparison is false.
     */
    TEST(CPreprocessorTest, UnsignedWinsInCondition) {
        static constexpr std::string_view src =
            "#if -1 < 0u\n#define A 0\n#else\n#define A 1\n#endif\nint x = A;";
        EXPECT_EQ(text_of(src, pp_tokens(src)), "int x = 1 ;");
    }

    TEST(CPreprocessorTest, ShortCircuitSuppressesDivisionByZero) {
        static constexpr std::string_view src = "#if 0 && 1 / 0\n#endif\n";
        CLexerBufferType                  buf = src;
        CLexer                            lx(buf);
        lx.scan();
        CPreprocessor pp(src);
        pp.process(lx.finalize());

        EXPECT_TRUE(none(pp.errors()));
    }

    TEST(CPreprocessorTest, UnterminatedIfIsAnError) {
        static constexpr std::string_view src = "#if 1\nint x;\n";
        CLexerBufferType                  buf = src;
        CLexer                            lx(buf);
        lx.scan();
        CPreprocessor pp(src);
        pp.process(lx.finalize());

        EXPECT_TRUE(has(pp.errors(), CPpErrorFlags::UnterminatedIf));
    }

    TEST(CPreprocessorTest, UnmatchedEndifIsAnError) {
        static constexpr std::string_view src = "int x;\n#endif\n";
        CLexerBufferType                  buf = src;
        CLexer                            lx(buf);
        lx.scan();
        CPreprocessor pp(src);
        pp.process(lx.finalize());

        EXPECT_TRUE(has(pp.errors(), CPpErrorFlags::UnmatchedEndif));
    }
}  // namespace Z::Zaban::Tests
