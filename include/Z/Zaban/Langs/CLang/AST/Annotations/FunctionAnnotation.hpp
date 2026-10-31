#pragma once

#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/AST/Parameter.hpp>
#include <utility>
#include <vector>

namespace Z::Zaban::Langs::CLang::AST::Annotations {
    template<typename OffsetType = std::size_t>
    class FunctionAnnotationNode
        : public Zaban::AST::AnnotationNode<OffsetType> {
        Zaban::AST::Annotation<OffsetType>             _return_type;
        std::vector<Zaban::AST::Parameter<OffsetType>> _parameters;

       public:
        FunctionAnnotationNode(
            Zaban::AST::Annotation<OffsetType>             return_type,
            std::vector<Zaban::AST::Parameter<OffsetType>> parameters) :
            _return_type(std::move(return_type)),
            _parameters(std::move(parameters)) {
        }

        Zaban::AST::AnnotationKind get_annotation_kind() const override {
            return Zaban::AST::AnnotationKind::Function;
        }
        Zaban::AST::Annotation<OffsetType> get_return_type() const {
            return _return_type;
        }
        const std::vector<Zaban::AST::Parameter<OffsetType>>& get_parameters()
            const {
            return _parameters;
        }
    };
}  // namespace Z::Zaban::Langs::CLang::AST::Annotations
