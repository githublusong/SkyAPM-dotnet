﻿ /*
 * Licensed to the SkyAPM under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The SkyAPM licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
 
using System;
using System.Data;
using System.Data.Common;
using System.Globalization;
using System.Linq;
using System.Reflection;
using System.Text;

namespace SkyApm.ClrProfiler.Trace.Utils
{
    /// <summary>
    ///     This API supports the Entity Framework Core infrastructure and is not intended to be used
    ///     directly from your code. This API may change or be removed in future releases.
    /// </summary>
    public static class DbParameterCollectionExtensions
    {
        /// <summary>
        ///     This API supports the Entity Framework Core infrastructure and is not intended to be used
        ///     directly from your code. This API may change or be removed in future releases.
        /// </summary>
        public static string FormatParameters(
            this DbParameterCollection parameters,
            bool logParameterValues)
        {
            return string.Join(",", parameters
                .Cast<DbParameter>()
                .Select(
                    p => FormatParameter(
                        p.ParameterName,
                        logParameterValues ? p.Value : "?",
                        logParameterValues,
                        p.Direction,
                        p.DbType,
                        p.IsNullable,
                        p.Size,
                        p.Precision,
                        p.Scale)));
        }

        public static bool IsNullableType(this Type type)
        {
            var typeInfo = type.GetTypeInfo();

            return !typeInfo.IsValueType
                   || typeInfo.IsGenericType
                   && typeInfo.GetGenericTypeDefinition() == typeof(Nullable<>);
        }

        /// <summary>
        ///     This API supports the Entity Framework Core infrastructure and is not intended to be used
        ///     directly from your code. This API may change or be removed in future releases.
        /// </summary>
        public static string FormatParameter(
            string name,
            object value,
            bool hasValue,
            ParameterDirection direction,
            DbType dbType,
            bool nullable,
            int size,
            byte precision,
            byte scale)
        {
            var builder = new StringBuilder();

            var clrType = value?.GetType();

            builder
                .Append(name)
                .Append("=");

            FormatParameterValue(builder, value);

            if (nullable
                && value != null
                && !clrType.IsNullableType())
            {
                builder.Append(" (Nullable = true)");
            }
            else
            {
                if (!nullable
                    && hasValue
                    && (value == null
                        || clrType.IsNullableType()))
                {
                    builder.Append(" (Nullable = false)");
                }
            }

            if (size != 0)
            {
                builder
                    .Append(" (Size = ")
                    .Append(size.ToString(CultureInfo.InvariantCulture))
                    .Append(')');
            }

            if (precision != 0)
            {
                builder
                    .Append(" (Precision = ")
                    .Append(precision.ToString(CultureInfo.InvariantCulture))
                    .Append(')');
            }

            if (scale != 0)
            {
                builder
                    .Append(" (Scale = ")
                    .Append(scale.ToString(CultureInfo.InvariantCulture))
                    .Append(')');
            }

            if (direction != ParameterDirection.Input)
            {
                builder
                    .Append(" (Direction = ")
                    .Append(direction)
                    .Append(')');
            }

            if (ShouldShowDbType(hasValue, dbType, clrType))
            {
                builder
                    .Append(" (DbType = ")
                    .Append(dbType)
                    .Append(')');
            }

            return builder.ToString();
        }

        private static void FormatParameterValue(StringBuilder builder, object parameterValue)
        {
            if (parameterValue == null)
            {
                builder.Append("''");
            }
            else if (parameterValue.GetType() == typeof(DateTime))
            {
                builder
                    .Append('\'')
                    .Append(((DateTime)parameterValue).ToString("s"))
                    .Append('\'');
            }
            else if (parameterValue.GetType() == typeof(DateTimeOffset))
            {
                builder
                    .Append('\'')
                    .Append(((DateTimeOffset)parameterValue).ToString("o"))
                    .Append('\'');
            }
            else if (parameterValue.GetType() == typeof(byte[]))
            {
                var buffer = (byte[])parameterValue;
                builder.Append("'0x");

                for (var i = 0; i < buffer.Length; i++)
                {
                    if (i > 31)
                    {
                        builder.Append("...");
                        break;
                    }

                    builder.Append(buffer[i].ToString("X2", CultureInfo.InvariantCulture));
                }

                builder.Append('\'');
            }
            else
            {
                var valueProperty = parameterValue.GetType().GetRuntimeProperty("Value");
                if (valueProperty != null
                    && valueProperty.PropertyType != parameterValue.GetType())
                {
                    var isNullProperty = parameterValue.GetType().GetRuntimeProperty("IsNull");
                    if (isNullProperty != null
                        && (bool)isNullProperty.GetValue(parameterValue))
                    {
                        builder.Append("''");
                    }
                    else
                    {
                        FormatParameterValue(builder, valueProperty.GetValue(parameterValue));
                    }
                }
                else
                {
                    builder
                        .Append('\'')
                        .Append(Convert.ToString(parameterValue, CultureInfo.InvariantCulture))
                        .Append('\'');
                }
            }
        }

        public static Type UnwrapNullableType(this Type type) => Nullable.GetUnderlyingType(type) ?? type;

        public static Type UnwrapEnumType(this Type type)
        {
            var isNullable = type.IsNullableType();
            var underlyingNonNullableType = isNullable ? type.UnwrapNullableType() : type;
            if (!underlyingNonNullableType.GetTypeInfo().IsEnum)
            {
                return type;
            }

            var underlyingEnumType = Enum.GetUnderlyingType(underlyingNonNullableType);
            return isNullable ? MakeNullable(underlyingEnumType) : underlyingEnumType;
        }

        public static Type MakeNullable(this Type type, bool nullable = true)
           => type.IsNullableType() == nullable
               ? type
               : nullable
                   ? typeof(Nullable<>).MakeGenericType(type)
                   : type.UnwrapNullableType();

        private static bool ShouldShowDbType(bool hasValue, DbType dbType, Type clrType)
        {
            if (!hasValue
                || clrType == null
                || clrType == typeof(DBNull))
            {
                return dbType != DbType.String;
            }

            clrType = clrType.UnwrapNullableType().UnwrapEnumType();

            switch (dbType)
            {
                case DbType.Binary:
                    return clrType != typeof(byte[]);
                case DbType.Byte:
                    return clrType != typeof(byte);
                case DbType.Boolean:
                    return clrType != typeof(bool);
                case DbType.Decimal:
                    return clrType != typeof(decimal);
                case DbType.Double:
                    return clrType != typeof(double);
                case DbType.Guid:
                    return clrType != typeof(Guid);
                case DbType.Int16:
                    return clrType != typeof(short);
                case DbType.Int32:
                    return clrType != typeof(int);
                case DbType.Int64:
                    return clrType != typeof(long);
                case DbType.Object:
                    return clrType != typeof(object);
                case DbType.SByte:
                    return clrType != typeof(sbyte);
                case DbType.Single:
                    return clrType != typeof(float);
                case DbType.String:
                    return clrType != typeof(string);
                case DbType.Time:
                    return clrType != typeof(TimeSpan);
                case DbType.UInt16:
                    return clrType != typeof(ushort);
                case DbType.UInt32:
                    return clrType != typeof(uint);
                case DbType.UInt64:
                    return clrType != typeof(ulong);
                case DbType.DateTime2:
                    return clrType != typeof(DateTime);
                case DbType.DateTimeOffset:
                    return clrType != typeof(DateTimeOffset);
                //case DbType.AnsiString:
                //case DbType.VarNumeric:
                //case DbType.AnsiStringFixedLength:
                //case DbType.StringFixedLength:
                //case DbType.Xml:
                //case DbType.Currency:
                //case DbType.Date:
                //case DbType.DateTime:
                default:
                    return true;
            }
        }
    }
}

