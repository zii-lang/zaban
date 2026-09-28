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
        class Prep {
           public:
            explicit Prep(std::string_view src) : _src(src), _pp(_src) {
            }

            Prep& define(std::string name, std::string value = "1") {
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
        Prep pp("let x = 42;");
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
        ZLexerBufferType buffer = "#if os\nx\n#end\ny";
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
     * Should: '#' 'if' 'os' is three, '#' 'end' is two.
     */
    TEST(ZPreprocessorTest, DirectiveLineMarked) {
        Prep pp("#if os\nx\n#end");
        pp.define("os");
        pp.run();

        EXPECT_EQ(count_flagged(pp.tokens(), TokenFlags::DirectiveLine), 5u)
            << describe(pp.tokens());
    }

    /**
     * Expect: leading whitespace does not stop a line being a directive.
     */
    TEST(ZPreprocessorTest, IndentedDirective) {
        Prep pp("  #if os\nx\n  #end");
        pp.define("os");
        pp.run();

        EXPECT_EQ(pp.code(), "x");
        EXPECT_EQ(pp.errors(), ZPpErrorFlags::None);
    }

    /**
     * Expect: a bare identifier is true when the config defines it.
     */
    TEST(ZPreprocessorTest, IfDefinedEmits) {
        Prep pp("#if os\nx\n#end");
        pp.define("os");
        pp.run();

        EXPECT_EQ(pp.code(), "x");
    }

    /**
     * Expect: an undefined name is false, and its branch is skipped.
     */
    TEST(ZPreprocessorTest, IfUndefinedSkips) {
        Prep pp("#if os\nx\n#end\ny");
        pp.run();

        EXPECT_EQ(pp.code(), "y");
        EXPECT_EQ(count_flagged(pp.tokens(), TokenFlags::Skipped), 1u)
            << describe(pp.tokens());
    }

    /**
     * Expect: #else runs when the #if did not.
     */
    TEST(ZPreprocessorTest, ElseTaken) {
        Prep pp("#if os\nx\n#else\ny\n#end");
        pp.run();

        EXPECT_EQ(pp.code(), "y");
    }

    /**
     * Expect: #else does not run when the #if did.
     */
    TEST(ZPreprocessorTest, ElseNotTaken) {
        Prep pp("#if os\nx\n#else\ny\n#end");
        pp.define("os");
        pp.run();

        EXPECT_EQ(pp.code(), "x");
    }

    /**
     * Expect: exactly one branch of a chain is taken, the first true one.
     */
    TEST(ZPreprocessorTest, ElifChainTakesFirstMatch) {
        Prep pp("#if os\na\n#elif arch\nc\n#elif vendor\ne\n#else\nf\n#end");
        pp.define("arch").define("vendor");
        pp.run();

        EXPECT_EQ(pp.code(), "c");
    }

    /**
     * Expect: a live outer branch with a dead inner one.
     */
    TEST(ZPreprocessorTest, NestedLiveOuterDeadInner) {
        Prep pp("#if os\np\n#if arch\nq\n#else\nr\n#end\n#end");
        pp.define("os");
        pp.run();

        EXPECT_EQ(pp.code(), "p r");
    }

    /**
     * Expect: nothing inside a dead outer branch emits, #else included.
     * Should: this is what BranchTaken on a level pushed while skipping buys.
     * Drop that flag and the inner #else reactivates the level.
     */
    TEST(ZPreprocessorTest, NestedDeadOuterLiveInner) {
        Prep pp("#if os\n#if arch\nq\n#else\nr\n#end\n#end\nz");
        pp.run();

        EXPECT_EQ(pp.code(), "z");
        EXPECT_EQ(pp.errors(), ZPpErrorFlags::None);
    }

    /**
     * Expect: a nested #if inside a dead branch still balances the stack.
     */
    TEST(ZPreprocessorTest, NestedInDeadBranchBalances) {
        Prep pp("#if os\n#if os\n#end\n#end\nz");
        pp.run();

        EXPECT_EQ(pp.code(), "z");
        EXPECT_EQ(pp.errors(), ZPpErrorFlags::None);
    }

    /**
     * Expect: == and != compare the resolved string values.
     */
    TEST(ZPreprocessorTest, Comparison) {
        Prep eq("#if arch == \"x86_64\"\nx\n#end");
        eq.define("arch", "x86_64");
        eq.run();
        EXPECT_EQ(eq.code(), "x");

        Prep neq("#if arch != \"x86_64\"\nx\n#end");
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
        Prep eq("#if os == \"x\"\na\n#end");
        eq.run();
        EXPECT_EQ(eq.code(), "");
        EXPECT_EQ(eq.errors(), ZPpErrorFlags::None);

        Prep neq("#if os != \"x\"\na\n#end");
        neq.run();
        EXPECT_EQ(neq.code(), "a");
    }

    /**
     * Expect: && binds tighter than ||.
     */
    TEST(ZPreprocessorTest, AndBindsTighterThanOr) {
        // false || (true && false) -> false
        Prep a("#if os || arch && env\nx\n#end");
        a.define("arch");
        a.run();
        EXPECT_EQ(a.code(), "");

        // (false && true) || true -> true
        Prep b("#if os && arch || env\nx\n#end");
        b.define("arch").define("env");
        b.run();
        EXPECT_EQ(b.code(), "x");
    }

    /**
     * Expect: parentheses override precedence.
     */
    TEST(ZPreprocessorTest, Parens) {
        // (false || true) && false -> false
        Prep pp("#if os || arch\nx\n#end\n#if (os || arch) && env\ny\n#end");
        pp.define("arch");
        pp.run();

        EXPECT_EQ(pp.code(), "x");
    }

    /**
     * Expect: ! negates, and nests.
     */
    TEST(ZPreprocessorTest, Not) {
        Prep pp("#if !os\nx\n#end\n#if !!os\ny\n#end");
        pp.run();

        EXPECT_EQ(pp.code(), "x");
    }

    /**
     * Expect: a short-circuited operand is still parsed.
     * Should: skip parsing the right side and the cursor desyncs, so the
     * trailing-junk check fires and this reports MalformedCondition.
     */
    TEST(ZPreprocessorTest, ShortCircuitStillParses) {
        Prep pp("#if os && arch\nx\n#end\n#if env || vendor\ny\n#end");
        pp.define("env").define("arch").define("vendor");
        pp.run();

        EXPECT_EQ(pp.code(), "y");
        EXPECT_EQ(pp.errors(), ZPpErrorFlags::None);
    }

    /**
     * Expect: a bare string is true when it is not empty.
     */
    TEST(ZPreprocessorTest, StringTruthiness) {
        Prep pp("#if \"s\"\nx\n#end\n#if \"\"\ny\n#end");
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
        Prep eq("#if arch == \"x86_64\"\nx\n#end");
        eq.define("arch", "x86_64");
        eq.run();
        EXPECT_EQ(eq.code(), "x");
        EXPECT_EQ(eq.errors(), ZPpErrorFlags::None);

        // Defining the payload as a name must not change the comparison.
        Prep shadowed("#if arch == \"x86_64\"\nx\n#end");
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
        Prep pp("#if arch == \"x86-64 && arm\"\nx\n#end");
        pp.define("arch", "x86-64 && arm");
        pp.run();

        EXPECT_EQ(pp.code(), "x");
        EXPECT_EQ(pp.errors(), ZPpErrorFlags::None);
    }

    /**
     * Expect: leftover tokens after a complete condition are an error.
     */
    TEST(ZPreprocessorTest, TrailingJunkIsMalformed) {
        Prep pp("#if os arch\nx\n#end");
        pp.define("os").define("arch");
        pp.run();

        EXPECT_TRUE(has(pp.errors(), ZPpErrorFlags::MalformedCondition));
        EXPECT_EQ(pp.code(), "") << "a malformed condition must fail closed";
    }

    /**
     * Expect: a missing operand and an empty condition are both malformed.
     */
    TEST(ZPreprocessorTest, MalformedConditions) {
        Prep dangling("#if os &&\nx\n#end");
        dangling.define("os");
        dangling.run();
        EXPECT_TRUE(has(dangling.errors(), ZPpErrorFlags::MalformedCondition));

        Prep empty("#if\nx\n#end");
        empty.run();
        EXPECT_TRUE(has(empty.errors(), ZPpErrorFlags::MalformedCondition));

        Prep unclosed("#if (os\nx\n#end");
        unclosed.define("os");
        unclosed.run();
        EXPECT_TRUE(has(unclosed.errors(), ZPpErrorFlags::MalformedCondition));
    }

    /**
     * Expect: a condition in a dead branch is never evaluated.
     * Should: so a malformed one there produces no diagnostic.
     */
    TEST(ZPreprocessorTest, DeadBranchConditionNotEvaluated) {
        Prep pp("#if os\n#if arch &&\n#end\n#end");
        pp.run();

        EXPECT_EQ(pp.errors(), ZPpErrorFlags::None) << describe(pp.tokens());
    }

    /**
     * Expect: each closer without an opener names its own error.
     */
    TEST(ZPreprocessorTest, UnmatchedClosers) {
        Prep end("#end");
        end.run();
        EXPECT_TRUE(has(end.errors(), ZPpErrorFlags::UnmatchedEnd));

        Prep els("#else");
        els.run();
        EXPECT_TRUE(has(els.errors(), ZPpErrorFlags::UnmatchedElse));

        Prep elif ("#elif a");
        elif.run();
        EXPECT_TRUE(has(elif.errors(), ZPpErrorFlags::UnmatchedElif));
    }

    /**
     * Expect: an #if with no #end is reported once per open level.
     */
    TEST(ZPreprocessorTest, UnterminatedIf) {
        Prep pp("#if os\nx\n#if arch\ny");
        pp.define("os").define("arch");
        pp.run();

        EXPECT_TRUE(has(pp.errors(), ZPpErrorFlags::UnterminatedIf));
        EXPECT_EQ(pp.diagnostics().size(), 2u);
    }

    /**
     * Expect: nothing may follow #else at the same level.
     */
    TEST(ZPreprocessorTest, ElseAfterElse) {
        Prep twice("#if os\nx\n#else\ny\n#else\nz\n#end");
        twice.define("os");
        twice.run();
        EXPECT_TRUE(has(twice.errors(), ZPpErrorFlags::ElseAfterElse));

        Prep elif ("#if os\nx\n#else\ny\n#elif arch\nz\n#end");
        elif.define("os").define("arch");
        elif.run();
        EXPECT_TRUE(has(elif.errors(), ZPpErrorFlags::ElseAfterElse));
    }

    /**
     * Expect: an unrecognised keyword is reported, with the spelling as arg.
     */
    TEST(ZPreprocessorTest, UnknownDirective) {
        Prep pp("#nope\nx");
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
        Prep pp("#if os\n#whatever\n#end");
        pp.run();

        EXPECT_EQ(pp.errors(), ZPpErrorFlags::None);
    }

    /**
     * Expect: all four condition keys are accepted.
     */
    TEST(ZPreprocessorTest, AllConditionKeysAccepted) {
        Prep pp("#if vendor && env && os && arch\nx\n#end");
        pp.define("vendor").define("env").define("os").define("arch");
        pp.run();

        EXPECT_EQ(pp.code(), "x");
        EXPECT_EQ(pp.errors(), ZPpErrorFlags::None);
    }

    /**
     * Expect: any other name is rejected, with the spelling as arg.
     * Should: fail closed, so the branch does not emit. Defining the name
     * changes nothing: the key set is a language rule, not a config lookup.
     */
    TEST(ZPreprocessorTest, UnknownConfigKeyRejected) {
        Prep pp("#if achr\nx\n#end\ny");
        pp.define("achr");
        pp.run();

        ASSERT_EQ(pp.diagnostics().size(), 1u);
        EXPECT_EQ(pp.diagnostics()[0].code, ZPpErrorFlags::UnKnownConfigKey);
        EXPECT_EQ(pp.diagnostics()[0].arg, "achr");
        EXPECT_EQ(pp.code(), "y");
    }

    /**
     * Expect: the rule applies to #elif too.
     */
    TEST(ZPreprocessorTest, UnknownConfigKeyInElif) {
        Prep pp("#if os\nx\n#elif nope\ny\n#end");
        pp.run();

        EXPECT_TRUE(has(pp.errors(), ZPpErrorFlags::UnKnownConfigKey));
        EXPECT_EQ(pp.code(), "");
    }

    /**
     * Expect: both sides of a comparison are checked.
     * Should: an unquoted right operand is a key, not a value, so it must be
     * one of the four. This forces 'os == "linux"'.
     */
    TEST(ZPreprocessorTest, UnknownConfigKeyOnRightOfComparison) {
        Prep pp("#if os == linux\nx\n#end");
        pp.define("os", "linux");
        pp.run();

        ASSERT_EQ(pp.diagnostics().size(), 1u);
        EXPECT_EQ(pp.diagnostics()[0].code, ZPpErrorFlags::UnKnownConfigKey);
        EXPECT_EQ(pp.diagnostics()[0].arg, "linux");
        EXPECT_EQ(pp.code(), "");
    }

    /**
     * Expect: a dead branch is not evaluated, so a bad key there is silent.
     */
    TEST(ZPreprocessorTest, UnknownConfigKeyInDeadBranch) {
        Prep pp("#if os\n#if nope\n#end\n#end");
        pp.run();

        EXPECT_EQ(pp.errors(), ZPpErrorFlags::None);
    }
    /**
     * Expect: ZPreprocessor reports through the same interface CLang's does.
     * Should: every ZLang code is an error today, so the view says Error and
     * the counts agree with the concrete diagnostics.
     */
    TEST(ZPreprocessorTest, DiagnosticViewsMatchConcreteDiagnostics) {
        ZLexerBufferType src = "#nope\nlet x = 1;";
        ZLexer           lexer(src);
        lexer.scan();

        ZPreprocessor                       pp(src);
        Pp::IPreprocessor<ZLexerTokenType>& iface = pp;
        iface.process(lexer.finalize());

        const std::vector<Zaban::Pp::PpDiagnosticView> views =
            iface.diagnostic_views();

        ASSERT_EQ(views.size(), 1u);
        ASSERT_EQ(views.size(), pp.diagnostics().size());

        EXPECT_EQ(views[0].severity, Lex::LexerDiagnosticSeverity::Error);
        EXPECT_EQ(views[0].range.begin, pp.diagnostics()[0].range.begin);
        EXPECT_EQ(views[0].range.end, pp.diagnostics()[0].range.end);
        EXPECT_EQ(views[0].arg, "nope");

        EXPECT_TRUE(iface.has_errors());
        EXPECT_EQ(iface.error_count(), 1u);
        EXPECT_EQ(iface.warning_count(), 0u);
    }

    TEST(ZPreprocessorTest, CleanSourceReportsNothingThroughInterface) {
        ZLexerBufferType src = "let x = 1;";
        ZLexer           lexer(src);
        lexer.scan();

        ZPreprocessor                       pp(src);
        Pp::IPreprocessor<ZLexerTokenType>& iface = pp;
        iface.process(lexer.finalize());

        EXPECT_FALSE(iface.has_errors());
        EXPECT_EQ(iface.error_count(), 0u);
        EXPECT_EQ(iface.warning_count(), 0u);
        EXPECT_TRUE(iface.diagnostic_views().empty());
    }
}  // namespace Z::Zaban::Tests
