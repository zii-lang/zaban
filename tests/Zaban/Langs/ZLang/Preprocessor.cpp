#include <gtest/gtest.h>

#include <Z/Zaban/Langs/ZLang/Lexer.hpp>
#include <Z/Zaban/Langs/ZLang/Preprocessor.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace Z::Zaban::Tests {
    using namespace Z::Zaban::Langs::ZLang;

    namespace {
        bool token_has(const ZLexerTokenType& t, TokenFlags f) {
            return has(static_cast<TokenFlags>(t.flags), f);
        }

        std::size_t count_flagged(const std::vector<ZLexerTokenType>& tokens,
                                  TokenFlags                          f) {
            std::size_t n = 0;
            for (const auto& t: tokens) {
                if (token_has(t, f)) ++n;
            }
            return n;
        }

        std::string describe(const std::vector<ZLexerTokenType>& tokens) {
            std::string out;
            for (std::size_t i = 0; i < tokens.size(); ++i) {
                if (i > 0) out += ", ";
                out += to_string(tokens[i].kind);
                if (token_has(tokens[i], TokenFlags::DirectiveLine)) out += "*";
                if (token_has(tokens[i], TokenFlags::Skipped)) out += "~";
            }
            return out;
        }

        /* Lexes then preprocesses, the way the driver will. Holds the buffer
           and the preprocessor as members so diagnostics outlive the call.
         */
        class Pp {
           public:
            explicit Pp(std::string_view src) : _src(src), _pp(_src) {
            }

            Pp& define(std::string name, std::string value = "1") {
                _config.define(std::move(name), std::move(value));
                return *this;
            }

            const std::vector<ZLexerTokenType>& run() {
                ZLexer lexer(_src);
                lexer.scan();
                _pp.set_config_source(_config);
                _tokens = _pp.process(lexer.finalize());
                return _tokens;
            }

            ZPpErrorFlags errors() const {
                return _pp.errors();
            }

            const std::vector<ZPPDiagnostic>& diagnostics() const {
                return _pp.diagnostics();
            }

            /// Spelling of what a compiler would see: no directives, nothing
            /// skipped. Ranges are inclusive, hence the + 1.
            std::string code() const {
                std::string out;
                for (const auto& t: _tokens) {
                    if (t.kind == ZLexerTokenKind::Eof) continue;
                    if (token_has(t, TokenFlags::DirectiveLine)) continue;
                    if (token_has(t, TokenFlags::Skipped)) continue;
                    if (!out.empty()) out += " ";
                    out += std::string(_src.substr(
                        t.range.begin, t.range.end - t.range.begin));
                }
                return out;
            }

            const std::vector<ZLexerTokenType>& tokens() const {
                return _tokens;
            }

           private:
            ZLexerBufferType             _src;
            MapConfigSource              _config;
            ZPreprocessor                _pp;
            std::vector<ZLexerTokenType> _tokens;
        };
    }  // namespace

    /**
     * Expect: source with no directives passes through untouched.
     */
    TEST(ZPreprocessorTest, NoDirectives) {
        Pp pp("let x = 42;");
        pp.run();

        EXPECT_EQ(pp.code(), "let x = 42 ;");
        EXPECT_EQ(count_flagged(pp.tokens(), TokenFlags::DirectiveLine), 0u);
        EXPECT_EQ(pp.errors(), ZPpErrorFlags::None);
    }

    /**
     * Expect: process() marks but never deletes.
     * Should: token count out equals token count in.
     */
    TEST(ZPreprocessorTest, PreservesStream) {
        ZLexerBufferType buffer = "#if nope\nx\n#end\ny";
        ZLexer           lexer(buffer);
        lexer.scan();
        const std::vector<ZLexerTokenType> before = lexer.finalize();

        MapConfigSource config;
        ZPreprocessor   pp(buffer);
        pp.set_config_source(config);
        const std::vector<ZLexerTokenType> after = pp.process(before);

        ASSERT_EQ(after.size(), before.size());
        for (std::size_t i = 0; i < before.size(); ++i) {
            EXPECT_EQ(after[i].kind, before[i].kind) << "index " << i;
            EXPECT_EQ(after[i].range.begin, before[i].range.begin);
        }
    }

    /**
     * Expect: every token on a directive line is flagged.
     * Should: '#' 'if' 'a' is three, '#' 'end' is two.
     */
    TEST(ZPreprocessorTest, DirectiveLineMarked) {
        Pp pp("#if a\nx\n#end");
        pp.define("a");
        pp.run();

        EXPECT_EQ(count_flagged(pp.tokens(), TokenFlags::DirectiveLine), 5u)
            << describe(pp.tokens());
    }

    /**
     * Expect: leading whitespace does not stop a line being a directive.
     */
    TEST(ZPreprocessorTest, IndentedDirective) {
        Pp pp("  #if a\nx\n  #end");
        pp.define("a");
        pp.run();

        EXPECT_EQ(pp.code(), "x");
        EXPECT_EQ(pp.errors(), ZPpErrorFlags::None);
    }

    /**
     * Expect: a bare identifier is true when the config defines it.
     */
    TEST(ZPreprocessorTest, IfDefinedEmits) {
        Pp pp("#if a\nx\n#end");
        pp.define("a");
        pp.run();

        EXPECT_EQ(pp.code(), "x");
    }

    /**
     * Expect: an undefined name is false, and its branch is skipped.
     */
    TEST(ZPreprocessorTest, IfUndefinedSkips) {
        Pp pp("#if nope\nx\n#end\ny");
        pp.run();

        EXPECT_EQ(pp.code(), "y");
        EXPECT_EQ(count_flagged(pp.tokens(), TokenFlags::Skipped), 1u)
            << describe(pp.tokens());
    }

    /**
     * Expect: #else runs when the #if did not.
     */
    TEST(ZPreprocessorTest, ElseTaken) {
        Pp pp("#if nope\nx\n#else\ny\n#end");
        pp.run();

        EXPECT_EQ(pp.code(), "y");
    }

    /**
     * Expect: #else does not run when the #if did.
     */
    TEST(ZPreprocessorTest, ElseNotTaken) {
        Pp pp("#if a\nx\n#else\ny\n#end");
        pp.define("a");
        pp.run();

        EXPECT_EQ(pp.code(), "x");
    }

    /**
     * Expect: exactly one branch of a chain is taken, the first true one.
     */
    TEST(ZPreprocessorTest, ElifChainTakesFirstMatch) {
        Pp pp("#if nope\na\n#elif b\nc\n#elif d\ne\n#else\nf\n#end");
        pp.define("b").define("d");
        pp.run();

        EXPECT_EQ(pp.code(), "c");
    }

    /**
     * Expect: a live outer branch with a dead inner one.
     */
    TEST(ZPreprocessorTest, NestedLiveOuterDeadInner) {
        Pp pp("#if a\np\n#if nope\nq\n#else\nr\n#end\n#end");
        pp.define("a");
        pp.run();

        EXPECT_EQ(pp.code(), "p r");
    }

    /**
     * Expect: nothing inside a dead outer branch emits, #else included.
     * Should: this is what BranchTaken on a level pushed while skipping buys.
     * Drop that flag and the inner #else reactivates the level.
     */
    TEST(ZPreprocessorTest, NestedDeadOuterLiveInner) {
        Pp pp("#if nope\n#if alsonope\nq\n#else\nr\n#end\n#end\nz");
        pp.run();

        EXPECT_EQ(pp.code(), "z");
        EXPECT_EQ(pp.errors(), ZPpErrorFlags::None);
    }

    /**
     * Expect: a nested #if inside a dead branch still balances the stack.
     */
    TEST(ZPreprocessorTest, NestedInDeadBranchBalances) {
        Pp pp("#if nope\n#if nope\n#end\n#end\nz");
        pp.run();

        EXPECT_EQ(pp.code(), "z");
        EXPECT_EQ(pp.errors(), ZPpErrorFlags::None);
    }

    /**
     * Expect: == and != compare the resolved string values.
     */
    TEST(ZPreprocessorTest, Comparison) {
        Pp eq("#if arch == \"x86_64\"\nx\n#end");
        eq.define("arch", "x86_64");
        eq.run();
        EXPECT_EQ(eq.code(), "x");

        Pp neq("#if arch != \"x86_64\"\nx\n#end");
        neq.define("arch", "x86_64");
        neq.run();
        EXPECT_EQ(neq.code(), "");
    }

    /**
     * Expect: an undefined name resolves to the empty string in a comparison.
     * Should: so '!=' against anything non-empty is true. Pinning the choice,
     * not asserting it is the only sensible one.
     */
    TEST(ZPreprocessorTest, UndefinedComparesAsEmpty) {
        Pp eq("#if nope == \"x\"\na\n#end");
        eq.run();
        EXPECT_EQ(eq.code(), "");
        EXPECT_EQ(eq.errors(), ZPpErrorFlags::None);

        Pp neq("#if nope != \"x\"\na\n#end");
        neq.run();
        EXPECT_EQ(neq.code(), "a");
    }

    /**
     * Expect: && binds tighter than ||.
     */
    TEST(ZPreprocessorTest, AndBindsTighterThanOr) {
        // false || (true && false) -> false
        Pp a("#if no1 || b && no2\nx\n#end");
        a.define("b");
        a.run();
        EXPECT_EQ(a.code(), "");

        // (false && true) || true -> true
        Pp b("#if no1 && b || c\nx\n#end");
        b.define("b").define("c");
        b.run();
        EXPECT_EQ(b.code(), "x");
    }

    /**
     * Expect: parentheses override precedence.
     */
    TEST(ZPreprocessorTest, Parens) {
        // (false || true) && false -> false
        Pp pp("#if no1 || b\nx\n#end\n#if (no1 || b) && no2\ny\n#end");
        pp.define("b");
        pp.run();

        EXPECT_EQ(pp.code(), "x");
    }

    /**
     * Expect: ! negates, and nests.
     */
    TEST(ZPreprocessorTest, Not) {
        Pp pp("#if !nope\nx\n#end\n#if !!nope\ny\n#end");
        pp.run();

        EXPECT_EQ(pp.code(), "x");
    }

    /**
     * Expect: a short-circuited operand is still parsed.
     * Should: skip parsing the right side and the cursor desyncs, so the
     * trailing-junk check fires and this reports MalformedCondition.
     */
    TEST(ZPreprocessorTest, ShortCircuitStillParses) {
        Pp pp("#if nope && b\nx\n#end\n#if a || c\ny\n#end");
        pp.define("a").define("b").define("c");
        pp.run();

        EXPECT_EQ(pp.code(), "y");
        EXPECT_EQ(pp.errors(), ZPpErrorFlags::None);
    }

    /**
     * Expect: a bare string is true when it is not empty.
     */
    TEST(ZPreprocessorTest, StringTruthiness) {
        Pp pp("#if \"s\"\nx\n#end\n#if \"\"\ny\n#end");
        pp.run();

        EXPECT_EQ(pp.code(), "x");
    }

    /**
     * Expect: a quoted value compares by its contents, not by its spelling.
     * Should: the quotes are stripped and the payload is never looked up as a
     * name. Pins that the operand is a String token rather than an identifier
     * that happens to sit between two quotes.
     */
    TEST(ZPreprocessorTest, QuotedOperandIsNotAName) {
        // 'x86_64' is a value here, never a defined name.
        Pp eq("#if arch == \"x86_64\"\nx\n#end");
        eq.define("arch", "x86_64");
        eq.run();
        EXPECT_EQ(eq.code(), "x");
        EXPECT_EQ(eq.errors(), ZPpErrorFlags::None);

        // Defining the payload as a name must not change the comparison.
        Pp shadowed("#if arch == \"x86_64\"\nx\n#end");
        shadowed.define("arch", "x86_64").define("x86_64", "something else");
        shadowed.run();
        EXPECT_EQ(shadowed.code(), "x");
    }

    /**
     * Expect: a quoted value may hold characters that would otherwise lex as
     * operators or directives.
     * Should: they stay inside the string and never reach the condition
     * parser, so the condition is well formed.
     */
    TEST(ZPreprocessorTest, QuotedOperandHoldsOperatorCharacters) {
        Pp pp("#if target == \"x86-64 && arm\"\nx\n#end");
        pp.define("target", "x86-64 && arm");
        pp.run();

        EXPECT_EQ(pp.code(), "x");
        EXPECT_EQ(pp.errors(), ZPpErrorFlags::None);
    }

    /**
     * Expect: leftover tokens after a complete condition are an error.
     */
    TEST(ZPreprocessorTest, TrailingJunkIsMalformed) {
        Pp pp("#if a b\nx\n#end");
        pp.define("a").define("b");
        pp.run();

        EXPECT_TRUE(has(pp.errors(), ZPpErrorFlags::MalformedCondition));
        EXPECT_EQ(pp.code(), "") << "a malformed condition must fail closed";
    }

    /**
     * Expect: a missing operand and an empty condition are both malformed.
     */
    TEST(ZPreprocessorTest, MalformedConditions) {
        Pp dangling("#if a &&\nx\n#end");
        dangling.define("a");
        dangling.run();
        EXPECT_TRUE(has(dangling.errors(), ZPpErrorFlags::MalformedCondition));

        Pp empty("#if\nx\n#end");
        empty.run();
        EXPECT_TRUE(has(empty.errors(), ZPpErrorFlags::MalformedCondition));

        Pp unclosed("#if (a\nx\n#end");
        unclosed.define("a");
        unclosed.run();
        EXPECT_TRUE(has(unclosed.errors(), ZPpErrorFlags::MalformedCondition));
    }

    /**
     * Expect: a condition in a dead branch is never evaluated.
     * Should: so a malformed one there produces no diagnostic.
     */
    TEST(ZPreprocessorTest, DeadBranchConditionNotEvaluated) {
        Pp pp("#if nope\n#if a &&\n#end\n#end");
        pp.run();

        EXPECT_EQ(pp.errors(), ZPpErrorFlags::None) << describe(pp.tokens());
    }

    /**
     * Expect: each closer without an opener names its own error.
     */
    TEST(ZPreprocessorTest, UnmatchedClosers) {
        Pp end("#end");
        end.run();
        EXPECT_TRUE(has(end.errors(), ZPpErrorFlags::UnmatchedEnd));

        Pp els("#else");
        els.run();
        EXPECT_TRUE(has(els.errors(), ZPpErrorFlags::UnmatchedElse));

        Pp elif ("#elif a");
        elif.run();
        EXPECT_TRUE(has(elif.errors(), ZPpErrorFlags::UnmatchedElif));
    }

    /**
     * Expect: an #if with no #end is reported once per open level.
     */
    TEST(ZPreprocessorTest, UnterminatedIf) {
        Pp pp("#if a\nx\n#if b\ny");
        pp.define("a").define("b");
        pp.run();

        EXPECT_TRUE(has(pp.errors(), ZPpErrorFlags::UnterminatedIf));
        EXPECT_EQ(pp.diagnostics().size(), 2u);
    }

    /**
     * Expect: nothing may follow #else at the same level.
     */
    TEST(ZPreprocessorTest, ElseAfterElse) {
        Pp twice("#if a\nx\n#else\ny\n#else\nz\n#end");
        twice.define("a");
        twice.run();
        EXPECT_TRUE(has(twice.errors(), ZPpErrorFlags::ElseAfterElse));

        Pp elif ("#if a\nx\n#else\ny\n#elif b\nz\n#end");
        elif.define("a").define("b");
        elif.run();
        EXPECT_TRUE(has(elif.errors(), ZPpErrorFlags::ElseAfterElse));
    }

    /**
     * Expect: an unrecognised keyword is reported, with the spelling as arg.
     */
    TEST(ZPreprocessorTest, UnknownDirective) {
        Pp pp("#nope\nx");
        pp.run();

        ASSERT_EQ(pp.diagnostics().size(), 1u);
        EXPECT_EQ(pp.diagnostics()[0].code, ZPpErrorFlags::UnknownDirective);
        EXPECT_EQ(pp.diagnostics()[0].arg, "nope");
        EXPECT_EQ(pp.code(), "x");
    }

    /**
     * Expect: unknown directives inside a dead branch are ignored.
     */
    TEST(ZPreprocessorTest, UnknownDirectiveInDeadBranch) {
        Pp pp("#if nope\n#whatever\n#end");
        pp.run();

        EXPECT_EQ(pp.errors(), ZPpErrorFlags::None);
    }
}  // namespace Z::Zaban::Tests
