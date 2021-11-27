#pragma once

#include <iostream>

#include <nlohmann/json.hpp>

#include <string>
#include <vector>
#include <map>
#include <functional>

template<typename T> struct argument_type;
template<typename T, typename U> struct argument_type<T(U)> { typedef U type; };


#define SerializableField(Type, Name)                                       \
    Core::Field<argument_type<void(Type)>::type> __FIELD__##Name = Core::Field<argument_type<void(Type)>::type>(this, Name, #Name); \
    argument_type<void(Type)>::type Name

    
#define SerializableField_2(Type, Name) UNPACK Type Name

#define SerializableField_S(Type, Name, Func)    Core::Field<argument_type<void(Type)>::type> __FIELD__##Name = Core::Field<argument_type<void(Type)>::type>(this, Name, #Name, Func); \
    argument_type<void(Type)>::type Name                                  

#define SerializableField_SD(Type, Name, FuncA, FuncB)             \
    Core::Field<argument_type<void(Type)>::type> __FIELD__##Name = Core::Field<argument_type<void(Type)>::type>(this, Name, #Name, FuncA, FuncB); \
    argument_type<void(Type)>::type Name                      


namespace Core
{
    enum class SerializerMode {
        Default,
        Unknown,
        Integral,
        Float
    };

    template <typename T, bool = std::is_fundamental<T>::value>
    struct Serializer
    {
    };

    template <typename T>
    struct Serializer<T, true>
    {
        static std::string Serialize(T object)
        {
            return std::to_string(object);
        }
    };
    template <typename T>
    struct Serializer<T, false>
    {
        static std::string Serialize(T object)
        {
            return object.Save();
        }
    };
    template <typename T>
    struct Serializer<std::vector<T>, false>
    {
        static std::string Serialize(std::vector<T> object)
        {
            nlohmann::json joutput;
            for(T& tobject : object)
            {
                joutput.push_back(Serializer<T>::Serialize(tobject));
            }
            return joutput.dump();
        }
    };
    template <typename T, typename F>
    struct Serializer<std::map<T, F>, false>
    {
        static std::string Serialize(std::map<T, F> object)
        {
            nlohmann::json joutput;
            for(auto &celement : object)
            {
                std::string key = Serializer<T>::Serialize(celement.first);
                std::string value = Serializer<F>::Serialize(celement.second);
                joutput[key] = value;
            }
            return joutput.dump();
        }
    };
    template<>
    struct Serializer<std::string, false>
    {
        static std::string Serialize(std::string object)
        {
            return object;
        }
    };
    template <typename T, SerializerMode mode = SerializerMode::Default>
    struct FundamentalDeserializer
    {
        static void Deserialize(std::string input, T &object)
        {
            if(std::is_integral<T>())
                return FundamentalDeserializer<T, SerializerMode::Integral>::Deserialize(input, object);
            else if(std::is_floating_point<T>())
                return FundamentalDeserializer<T, SerializerMode::Float>::Deserialize(input, object);
        }
    };
    template <typename T>
    struct FundamentalDeserializer<T, SerializerMode::Integral>
    {
        static void Deserialize(std::string input, T& object)
        {
            long long int i = std::atoll(input.c_str());
            object = T(i);
        }
    };
    template <typename T>
    struct FundamentalDeserializer<T, SerializerMode::Float>
    {
        static void Deserialize(std::string input, T& object)
        {
            long double i = std::stold(input.c_str());
            object = T(i);
        }

    };
    template <typename T, bool = std::is_fundamental<T>::value>
    struct Deserializer
    {
    };

    template <typename T>
    struct Deserializer<T, false>
    {
        static void Deserialize(std::string input, T &object)
        {
            object.Load(input);
        }
    };
    template <typename T>
    struct Deserializer<std::vector<T>, false>
    {
        static void Deserialize(std::string input, std::vector<T> &object)
        {
            object.clear();
            nlohmann::json jinput = nlohmann::json::parse(input);
            
            for(nlohmann::json::iterator it = jinput.begin(); it != jinput.end(); ++it)
            {
                std::string celement = (*it).dump();
                T tobject;
                Deserializer<T>::Deserialize(celement, tobject);
                object.push_back(tobject);
            }
        }
    };
    template <typename T, typename F>
    struct Deserializer<std::map<T, F>, false>
    {
        static void Deserialize(std::string input, std::map<T, F> &object)
        {
            object.clear();
            nlohmann::json jinput = nlohmann::json::parse(input);

            for(nlohmann::json::iterator it = jinput.begin(); it != jinput.end(); ++it)
            {
                T tkey;
                F fvalue;
                Deserializer<T>::Deserialize(it.key(), tkey);
                Deserializer<F>::Deserialize(it.value(), fvalue);
                object[tkey] = fvalue;
            }
        }
    };
    template<>
    struct Deserializer<std::string, false>
    {
        static void Deserialize(std::string input, std::string &object)
        {
            object = input;
        }
    };
    template <typename T>
    struct Deserializer<T, true>
    {
        static void Deserialize(std::string input, T &object)
        {
            FundamentalDeserializer<T, SerializerMode::Default>::Deserialize(input, object);
        }
    };

    // Cool to have: std::string, std::vector, std::map, std::list, Vectors?

    class FieldBase
    {
    public:
        virtual std::string GetName() = 0;
        virtual std::string Save() = 0;
        virtual void Load(std::string input) = 0;
        ~FieldBase() {}
    };

    class Serializable
    {
    public:
        void RegisterField(FieldBase *ptr)
        {
            FieldList.push_back(ptr);
        }
        virtual std::string Save()
        {
            nlohmann::json joutput;
            for (auto &f : FieldList)
            {
                joutput[f->GetName()] = f->Save();
            }
            std::string output = joutput.dump();
            return output;
        }
        virtual void Load(std::string input)
        {
            nlohmann::json jinput = nlohmann::json::parse(input);
            for (auto &f : FieldList)
            {
                std::string Content = jinput[f->GetName()];
                f->Load(Content);
            }
        }

    private:
        std::vector<FieldBase *> FieldList;
    };
    template <typename T>
    class Field : public FieldBase
    {
    public:
        Field(Serializable *caller,
              T &_object, std::string _name,
              std::function<std::string(T)> _SerializerFunctor = Serializer<T>::Serialize,
              std::function<void(std::string, T &)> _DeserializerFunctor = Deserializer<T>::Deserialize)
            : object(_object), name(_name), SerializerFunctor(_SerializerFunctor), DeserializerFunctor(_DeserializerFunctor)
        {
            caller->RegisterField(this);
        }
        std::string GetName()
        {
            return name;
        }
        std::string Save()
        {
            return SerializerFunctor(object);
        }
        void Load(std::string input)
        {
            DeserializerFunctor(input, object);
        }

    private:
        std::function<std::string(T)> SerializerFunctor;
        std::function<void(std::string, T &)> DeserializerFunctor;
        T &object;
        const std::string name;
    };
}