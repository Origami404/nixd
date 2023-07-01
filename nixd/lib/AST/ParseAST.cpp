#include "nixd/AST/ParseAST.h"

namespace nixd {

std::optional<ParseAST::Definition>
ParseAST::lookupDef(lspserver::Position Desired) const {
  for (const auto &[Def, _] : References) {
    try {
      if (lspserver::Range(defRange(Def)).contains(Desired))
        return Def;
    } catch (std::out_of_range &) {
    }
  }
  return std::nullopt;
}

[[nodiscard]] const nix::Expr *
ParseAST::lookupEnd(lspserver::Position Desired) const {
  struct VTy : RecursiveASTVisitor<VTy> {
    const ParseAST &This;

    const nix::Expr *R;
    lspserver::Position RStart = {INT_MAX, INT_MAX};
    lspserver::Position REnd = {INT_MIN, INT_MIN};

    const lspserver::Position Desired;

    bool visitExpr(const nix::Expr *E) {
      auto ER = This.lRange(E);
      if (ER.has_value() && ER->end < Desired &&
          (REnd < ER->end || (REnd == ER->end && RStart <= ER->start))) {
        // Update the expression.
        R = E;
        RStart = ER->start;
        REnd = ER->end;
      }
      return true;
    }
  } V{.This = *this, .Desired = Desired};

  V.traverseExpr(root());
  return V.R;
}

std::vector<const nix::Expr *>
ParseAST::lookupContain(lspserver::Position Desired) const {
  struct VTy : RecursiveASTVisitor<VTy> {
    const ParseAST &This;

    std::vector<const nix::Expr *> R;

    const lspserver::Position Desired;

    bool visitExpr(const nix::Expr *E) {
      auto ER = This.lRange(E);
      if (ER.has_value() && ER->contains(Desired))
        R.emplace_back(E);
      return true;
    }
  } V{.This = *this, .Desired = Desired};

  V.traverseExpr(root());
  return V.R;
}

[[nodiscard]] const nix::Expr *
ParseAST::lookupContainMin(lspserver::Position Desired) const {
  struct VTy : RecursiveASTVisitor<VTy> {
    const ParseAST &This;

    const nix::Expr *R;
    lspserver::Range RR = {{INT_MIN, INT_MIN}, {INT_MAX, INT_MAX}};

    const lspserver::Position Desired;

    bool visitExpr(const nix::Expr *E) {
      auto ER = This.lRange(E);
      if (ER.has_value() && ER->contains(Desired) && RR.contains(*ER)) {
        // Update the expression.
        R = E;
        RR = *ER;
      }
      return true;
    }

  } V{.This = *this, .Desired = Desired};

  V.traverseExpr(root());
  return V.R;
}

[[nodiscard]] const nix::Expr *
ParseAST::lookupStart(lspserver::Position Desired) const {
  struct VTy : RecursiveASTVisitor<VTy> {
    const ParseAST &This;

    const nix::Expr *R;
    lspserver::Position RStart = {INT_MAX, INT_MAX};
    lspserver::Position REnd = {INT_MIN, INT_MIN};

    const lspserver::Position Desired;

    bool visitExpr(const nix::Expr *E) {
      auto ER = This.lRange(E);
      if (ER.has_value() && Desired <= ER->start &&
          (ER->start < RStart || (ER->start == RStart && ER->end <= REnd))) {
        // Update the expression.
        R = E;
        RStart = ER->start;
        REnd = ER->end;
      }
      return true;
    }
  } V{.This = *this, .Desired = Desired};

  V.traverseExpr(root());
  return V.R;
}

void ParseAST::prepareDefRef() {
  struct VTy : RecursiveASTVisitor<VTy> {
    ParseAST &This;
    bool visitExprVar(const nix::ExprVar *E) {
      if (E->fromWith)
        return true;
      auto Def = This.searchDef(E);
      if (Def) {
        This.Definitions[E] = *Def;
        This.References[*Def].emplace_back(E);
      }
      return true;
    }
  } V{.This = *this};
  V.traverseExpr(root());
}

std::optional<ParseAST::Definition>
ParseAST::searchDef(const nix::ExprVar *Var) const {
  if (Var->fromWith)
    return std::nullopt;
  const auto *EnvExpr = envExpr(Var);
  if (EnvExpr)
    return Definition{EnvExpr, Var->displ};
  return std::nullopt;
}

} // namespace nixd