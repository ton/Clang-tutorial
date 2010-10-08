#include <iostream>

#include "llvm/Config/config.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/System/Host.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/FileManager.h"

#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/Preprocessor.h"

#include "clang/Frontend/DiagnosticOptions.h"
#include "clang/Frontend/FrontendOptions.h"
#include "clang/Frontend/HeaderSearchOptions.h"
#include "clang/Frontend/PreprocessorOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/Utils.h"

#include "clang/Parse/Parser.h"

#include "clang/Sema/Sema.h"


int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: ./tutorial <CXX source file>" << std::endl;
        return 0;
    }

    clang::DiagnosticOptions diagnosticOptions;
    clang::TextDiagnosticPrinter *textDiagnosticPrinter = new clang::TextDiagnosticPrinter(llvm::outs(), diagnosticOptions);
    clang::Diagnostic diagnostic(textDiagnosticPrinter);

    clang::LangOptions langOptions;

    clang::TargetOptions targetOptions;
    targetOptions.Triple = llvm::sys::getHostTriple();
    clang::TargetInfo *targetInfo = clang::TargetInfo::CreateTargetInfo(diagnostic, targetOptions);

    clang::SourceManager sourceManager(diagnostic);

    clang::FileManager fileManager;

    clang::HeaderSearch headerSearch(fileManager);
    clang::HeaderSearchOptions headerSearchOptions;
    // --- Warning: platform dependent code. 
    headerSearchOptions.AddPath("/usr/include/linux", clang::frontend::Angled, false, false, false);
    headerSearchOptions.AddPath("/usr/include/c++/4.4/tr1", clang::frontend::Angled, false, false, false);
    headerSearchOptions.AddPath("/usr/include/c++/4.4/bits", clang::frontend::Angled, false, false, false);
    headerSearchOptions.AddPath("/usr/include/c++/4.4", clang::frontend::Angled, false, false, false);
    // --- End platform dependent code
    clang::ApplyHeaderSearchOptions(headerSearch, headerSearchOptions, langOptions, targetInfo->getTriple());

    clang::Preprocessor preProcessor(diagnostic, langOptions, *targetInfo, sourceManager, headerSearch);

    clang::PreprocessorOptions preprocessorOptions;
    clang::FrontendOptions frontendOptions;
    clang::InitializePreprocessor(preProcessor, preprocessorOptions, headerSearchOptions, frontendOptions);

    const clang::FileEntry *file = fileManager.getFile(argv[1]);
    sourceManager.createMainFileID(file);
    preProcessor.EnterMainSourceFile();

    clang::Token token;
    do
    {
        preProcessor.Lex(token);
        if (diagnostic.hasErrorOccurred())
        {
            break;
        }
        preProcessor.DumpToken(token);
        std::cerr << std::endl;
    }
    while (token.isNot(clang::tok::eof));

    clang::IdentifierTable identifierTable(langOptions);
    clang::SelectorTable selectorTable;
    clang::Builtin::Context builtins(*targetInfo);
    clang::ASTContext astContext(langOptions, sourceManager, *targetInfo, identifierTable, selectorTable, builtins, 0);
    clang::ASTConsumer astConsumer;

    clang::Sema sema(preProcessor, astContext, astConsumer);
    clang::Parser parser(preProcessor, sema);
    parser.ParseTranslationUnit();

    identifierTable.PrintStats();

    return 0;
}
