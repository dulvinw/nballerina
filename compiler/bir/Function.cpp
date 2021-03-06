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
#include "bir/Package.h"
#include "bir/Types.h"
#include <algorithm>
#include <cassert>

namespace nballerina {

Function::Function(Package *parentPackage, std::string name, std::string workerName, unsigned int flags)
    : parentPackage(parentPackage), name(std::move(name)), workerName(std::move(workerName)), flags(flags) {}

const std::string &Function::getName() const { return name; }
const std::vector<FunctionParam> &Function::getParams() const { return requiredParams; }
const std::optional<RestParam> &Function::getRestParam() const { return restParam; }
const std::optional<Variable> &Function::getReturnVar() const { return returnVar; }

const Variable &Function::getLocalOrGlobalVariable(const Operand &op) const {
    if (op.getKind() == GLOBAL_VAR_KIND) {
        return parentPackage->getGlobalVariable(op.getName());
    }
    return getLocalVariable(op.getName());
}

const Variable &Function::getLocalVariable(const std::string &opName) const {
    auto result = std::find_if(localVars.begin(), localVars.end(),
                               [&opName](const Variable &i) -> bool { return i.getName() == opName; });
    assert(result != localVars.end());
    return *result;
}

size_t Function::getNumParams() const { return requiredParams.size(); }

bool Function::isMainFunction() const { return (name == MAIN_FUNCTION_NAME); }

bool Function::isExternalFunction() const { return ((flags & NATIVE) == NATIVE); }

} // namespace nballerina
