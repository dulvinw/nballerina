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

#ifndef __FUNCTIONCALLINSN__H__
#define __FUNCTIONCALLINSN__H__

#include "bir/RestParam.h"
#include "interfaces/TerminatorInsn.h"
#include <string>
#include <vector>

namespace nballerina {

class FunctionCallInsn : public TerminatorInsn, public Translatable<FunctionCallInsn> {
  private:
    std::string functionName;
    int argCount;
    std::vector<Operand> argsList;

  public:
    FunctionCallInsn(BasicBlock &currentBB, std::string thenBBID, Operand lhs, std::string functionName, int argCount,
                     std::vector<Operand> argsList)
        : TerminatorInsn(std::move(lhs), currentBB, std::move(thenBBID)), functionName(std::move(functionName)),
          argCount(argCount), argsList(std::move(argsList)) {
        kind = INSTRUCTION_KIND_CALL;
    }

    friend class TerminatorInsnCodeGen;
};

} // namespace nballerina

#endif //!__FUNCTIONCALLINSN__H__
