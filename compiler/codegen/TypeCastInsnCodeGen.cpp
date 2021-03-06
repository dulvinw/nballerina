/*
 * Copyright (c) 2021, WSO2 Inc. (http://www.wso2.org) All Rights Reserved.
 *
 * WSO2 Inc. licenses this file to you under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "bir/Function.h"
#include "bir/Operand.h"
#include "bir/TypeCastInsn.h"
#include "bir/Types.h"
#include "bir/Variable.h"
#include "codegen/CodeGenUtils.h"
#include "codegen/NonTerminatorInsnCodeGen.h"

namespace nballerina {

void NonTerminatorInsnCodeGen::visit(class TypeCastInsn &obj, llvm::IRBuilder<> &builder) {
    const auto &funcObj = obj.getFunctionRef();
    auto *rhsOpRef = functionGenerator.getLocalOrGlobalVal(obj.rhsOp);
    auto *lhsOpRef = functionGenerator.getLocalOrGlobalVal(obj.lhsOp);
    auto *lhsTypeRef = lhsOpRef->getType();

    const auto &lhsVar = funcObj.getLocalOrGlobalVariable(obj.lhsOp);
    const auto &rhsVar = funcObj.getLocalOrGlobalVariable(obj.rhsOp);
    const auto &lhsType = lhsVar.getType();
    auto lhsTypeTag = lhsType.getTypeTag();
    const auto &rhsType = rhsVar.getType();
    auto rhsTypeTag = rhsType.getTypeTag();

    if (Type::isSmartStructType(rhsTypeTag)) {
        if (Type::isSmartStructType(lhsTypeTag)) {
            auto *rhsVarOpRef = functionGenerator.createTempVal(obj.rhsOp, builder);
            builder.CreateStore(rhsVarOpRef, lhsOpRef);
            return;
        }
        // GEP of last type of smart pointer(original type of any variable(smart pointer))
        auto *inherentTypeStringPtr = builder.CreateStructGEP(rhsOpRef, 0, "inherentTypeName");
        auto *inherentTypeStringVal = builder.CreateLoad(inherentTypeStringPtr);
        auto *dataPtr = builder.CreateStructGEP(rhsOpRef, 1, "data");
        auto *dataVal = builder.CreateLoad(dataPtr);

        // get the mangled name of the lhs type and store it to string builder table.
        std::string_view lhsTypeName = Type::typeStringMangleName(lhsType);
        auto *lhsGep = moduleGenerator.addToStringTable(lhsTypeName, builder);

        // call is_same_type rust function to check LHS and RHS type are same or not.
        auto &module = moduleGenerator.getModule();
        auto isSameTypeFunc = CodeGenUtils::getIsSameTypeFunc(module, lhsGep, inherentTypeStringVal);
        auto *sameTypeResult =
            builder.CreateCall(isSameTypeFunc, llvm::ArrayRef<llvm::Value *>({lhsGep, inherentTypeStringVal}));
        auto *compareVal = builder.CreateIsNotNull(sameTypeResult);

        // creating branch condition using is_same_type() function result.
        auto *functionVal = functionGenerator.getFunctionValue();
        auto &context = module.getContext();
        auto *ifBB = llvm::BasicBlock::Create(context, "bb.ok", functionVal);
        auto *elseBB = llvm::BasicBlock::Create(context, "bb.abort", functionVal);
        builder.CreateCondBr(compareVal, ifBB, elseBB);

        // if comparison failed
        builder.SetInsertPoint(elseBB);
        auto abortFunc = CodeGenUtils::getAbortFunc(module);
        auto *abortCall = builder.CreateCall(abortFunc);
        abortCall->addAttribute(~0U, llvm::Attribute::NoReturn);
        abortCall->addAttribute(~0U, llvm::Attribute::NoUnwind);
        builder.CreateUnreachable();

        // if comparison is successful
        builder.SetInsertPoint(ifBB);
        auto *castResult = builder.CreateBitCast(dataVal, lhsTypeRef, obj.lhsOp.getName());
        auto *castLoad = builder.CreateLoad(castResult);
        builder.CreateStore(castLoad, lhsOpRef);

    } else if (Type::isSmartStructType(lhsTypeTag)) {
        moduleGenerator.storeValueInSmartStruct(builder, rhsOpRef, rhsType, lhsOpRef);
    } else if (lhsTypeTag == TYPE_TAG_INT && rhsTypeTag == TYPE_TAG_FLOAT) {
        auto *rhsLoad = builder.CreateLoad(rhsOpRef);
        auto *lhsLoad = builder.CreateLoad(lhsOpRef);
        auto *castResult = builder.CreateFPToSI(rhsLoad, lhsLoad->getType(), "");
        builder.CreateStore(castResult, lhsOpRef);
    } else {
        builder.CreateBitCast(rhsOpRef, lhsTypeRef, "data_cast");
    }
}

} // namespace nballerina
