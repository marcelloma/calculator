#include <libgccjit.h>
#include <json-c/json.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define STRUCTURE_FILE "./structure.json"

typedef struct
{
  gcc_jit_context *ctx;
  gcc_jit_type *bool_type;
  gcc_jit_type *int_type;
  gcc_jit_type *double_type;
  gcc_jit_type *double_ptr_type;
  gcc_jit_type *string_type;
  gcc_jit_type *money_type;
  gcc_jit_type *money_ptr_type;
  gcc_jit_field *money_amount_field;
  gcc_jit_field *money_currency_field;
} compiler_env;

gcc_jit_rvalue *compile_un_op_ast(gcc_jit_context *ctx, const char *token, json_object *ast);
gcc_jit_rvalue *compile_bin_op_ast(gcc_jit_context *ctx, const char *token, json_object *lhs_ast, json_object *rhs_ast);
gcc_jit_rvalue *compile_ast(gcc_jit_context *ctx, json_object *ast);

gcc_jit_rvalue *compile_un_op_ast(gcc_jit_context *ctx, const char *token, json_object *ast)
{
  gcc_jit_type *jit_int_type = gcc_jit_context_get_type(ctx, GCC_JIT_TYPE_INT);
  gcc_jit_rvalue *rvalue = compile_ast(ctx, ast);

  if (strcmp(token, "-") == 0)
  {
    return gcc_jit_context_new_unary_op(ctx, NULL, GCC_JIT_UNARY_OP_MINUS, jit_int_type, rvalue);
  }
  if (strcmp(token, "-") == 0)
  {
    return gcc_jit_context_new_unary_op(ctx, NULL, GCC_JIT_UNARY_OP_MINUS, jit_int_type, rvalue);
  }
  else
  {
    fprintf(stderr, "unsupported unary operator %s", token);
    exit(1);
  }
}

gcc_jit_rvalue *compile_bin_op_ast(gcc_jit_context *ctx, const char *token, json_object *lhs_ast, json_object *rhs_ast)
{
  gcc_jit_type *jit_int_type = gcc_jit_context_get_type(ctx, GCC_JIT_TYPE_INT);
  gcc_jit_rvalue *lhs_rvalue = compile_ast(ctx, lhs_ast);
  gcc_jit_rvalue *rhs_rvalue = compile_ast(ctx, rhs_ast);

  if (strcmp(token, "+") == 0)
  {
    return gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_PLUS, jit_int_type, lhs_rvalue, rhs_rvalue);
  }
  else if (strcmp(token, "-") == 0)
  {
    return gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MINUS, jit_int_type, lhs_rvalue, rhs_rvalue);
  }
  else if (strcmp(token, "*") == 0)
  {
    return gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_MULT, jit_int_type, lhs_rvalue, rhs_rvalue);
  }
  else if (strcmp(token, "/") == 0)
  {
    return gcc_jit_context_new_binary_op(ctx, NULL, GCC_JIT_BINARY_OP_DIVIDE, jit_int_type, lhs_rvalue, rhs_rvalue);
  }
  else
  {
    fprintf(stderr, "unsupported binary operator %s", token);
    exit(1);
  }
}

gcc_jit_rvalue *compile_ast(gcc_jit_context *ctx, json_object *ast)
{
  json_type json_obj_type = json_object_get_type(ast);

  switch (json_obj_type)
  {
  case json_type_int:
  {
    gcc_jit_type *jit_int_type = gcc_jit_context_get_type(ctx, GCC_JIT_TYPE_INT);
    int32_t value = json_object_get_int(ast);
    return gcc_jit_context_new_rvalue_from_int(ctx, jit_int_type, value);
  }

  case json_type_array:
  {
    size_t ast_size = json_object_array_length(ast) - 2;
    json_object *token = json_object_array_get_idx(ast, 0);
    const char *token_str = json_object_get_string(token);

    switch (ast_size)
    {
    case 1:
    {
      json_object *arg_ast = json_object_array_get_idx(ast, 1);
      return compile_un_op_ast(ctx, token_str, arg_ast);
    }
    case 2:
    {
      json_object *lhs_ast = json_object_array_get_idx(ast, 1);
      json_object *rhs_ast = json_object_array_get_idx(ast, 2);
      return compile_bin_op_ast(ctx, token_str, lhs_ast, rhs_ast);
    }
    default:
    {
      fprintf(stderr, "unsupported ast");
      exit(1);
    }
    }
  }
  }

  return 0;
}

// void compile_object(gcc_jit_context *ctx, json_object *obj)
// {
//   gcc_jit_type *jit_int_type = gcc_jit_context_get_type(ctx, GCC_JIT_TYPE_INT);

//   gcc_jit_function *func =
//       gcc_jit_context_new_function(
//           ctx, NULL, GCC_JIT_FUNCTION_EXPORTED, jit_int_type, "entry", 0, NULL, 0);

//   gcc_jit_block *block = gcc_jit_function_new_block(func, NULL);

//   gcc_jit_block_end_with_return(block, NULL, compile_ast(ctx, root_ast));
// }

json_object *read_structure_json()
{
  FILE *file = fopen(STRUCTURE_FILE, "r");

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *file_contents = malloc(file_size + 1);
  fread(file_contents, file_size, 1, file);

  json_object *structure = json_tokener_parse(file_contents);
  free(file_contents);
  fclose(file);

  return structure;
}

compiler_env *build_env()
{
  compiler_env *env = malloc(sizeof(compiler_env));
  if (env == NULL)
    exit(1);

  env->ctx = gcc_jit_context_acquire();
  env->bool_type = gcc_jit_context_get_type(env->ctx, GCC_JIT_TYPE_BOOL);
  env->int_type = gcc_jit_context_get_type(env->ctx, GCC_JIT_TYPE_INT);
  env->double_type = gcc_jit_context_get_type(env->ctx, GCC_JIT_TYPE_DOUBLE);
  env->double_ptr_type = gcc_jit_type_get_pointer(env->double_type);
  env->string_type = gcc_jit_context_get_type(env->ctx, GCC_JIT_TYPE_CONST_CHAR_PTR);  

  env->money_amount_field = gcc_jit_context_new_field(env->ctx, NULL, env->double_type, "amount");
  env->money_currency_field = gcc_jit_context_new_field(env->ctx, NULL, env->string_type, "currency");
  gcc_jit_field *money_fields[2] = {env->money_amount_field, env->money_currency_field};
  gcc_jit_struct *money = gcc_jit_context_new_struct_type(env->ctx, NULL, "money", 2, money_fields);
  
  env->money_type = gcc_jit_struct_as_type(money);
  env->money_ptr_type = gcc_jit_type_get_pointer(env->money_type);

  return env;
}

void test_money(compiler_env *env)
{
  gcc_jit_function *func =
      gcc_jit_context_new_function(
          env->ctx, NULL, GCC_JIT_FUNCTION_EXPORTED, env->money_ptr_type, "entry", 0, NULL, 0);

  gcc_jit_block *block = gcc_jit_function_new_block(func, NULL);

  gcc_jit_lvalue *money_lvalue = gcc_jit_function_new_local(func, NULL, env->money_type, "money");
  gcc_jit_lvalue *money_amount_lvalue = gcc_jit_lvalue_access_field(money_lvalue, NULL, env->money_amount_field);
  gcc_jit_rvalue *money_amount_rvalue = gcc_jit_context_new_rvalue_from_double(env->ctx, env->double_type, 20.0);
  gcc_jit_lvalue *money_currency_lvalue = gcc_jit_lvalue_access_field(money_lvalue, NULL, env->money_currency_field);
  gcc_jit_rvalue *money_currency_rvalue = gcc_jit_context_new_rvalue_from_ptr(env->ctx, env->string_type, "USD");
  gcc_jit_block_add_assignment(block, NULL, money_amount_lvalue, money_amount_rvalue);
  gcc_jit_block_add_assignment(block, NULL, money_currency_lvalue, money_currency_rvalue);
  gcc_jit_rvalue *money_ptr_rvalue = gcc_jit_lvalue_get_address(money_lvalue, NULL);
  // gcc_jit_block_end_with_return(block, NULL, gcc_jit_lvalue_as_rvalue(money_lvalue));
  gcc_jit_block_end_with_return(block, NULL, money_ptr_rvalue);

  gcc_jit_result *result = gcc_jit_context_compile(env->ctx);
  if (!result)
    {
      fprintf (stderr, "NULL result");
      exit (1);
    }

  typedef struct{
    double amount;
    const char* currency;
  }money;
  typedef money* (*entry_fn) ();
  entry_fn entry = (entry_fn)gcc_jit_result_get_code(result, "entry");
  
  if (!entry)
    {
      fprintf (stderr, "NULL entry");
      exit (1);
    }

  money *returned_money = entry();
  // money returned_money = entry();

  printf("%f %s\n", returned_money->amount, returned_money->currency);
  printf("%f %s\n", returned_money->amount, returned_money->currency);
  printf("%f %s\n", returned_money->amount, returned_money->currency);
  // printf("%f %s\n", returned_money.amount, returned_money.currency);
  // printf("%f %s\n", returned_money.amount, returned_money.currency);
  // printf("%f %s\n", returned_money.amount, returned_money.currency);
  
  gcc_jit_result_release(result);
}


void release_env(compiler_env *env)
{
  gcc_jit_context_release(env->ctx);

  free(env);
}

int main(int argc, char **argv)
{
  compiler_env *env = build_env();

  json_object *structure_json = read_structure_json();

  test_money(env);

  // printf("%d.%d.%d", gcc_jit_version_major(), gcc_jit_version_minor(), gcc_jit_version_patchlevel());

  release_env(env);

  return 0;
}
