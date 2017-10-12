
#ifndef __RSSL_QOS_H
#define __RSSL_QOS_H


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTypes.h"
#include "rtr/rsslRetCodes.h"
#include "rtr/rsslDataUtils.h"

/** 
 * @addtogroup RsslQosType
 * @{
 */

/** 
 * @brief The RsslQos can be used to indicate the quality of service associated with content.  This includes timeliness (e.g. age) and rate (e.g. period of change) information.
 * @see RSSL_INIT_QOS, rsslClearQos, rsslEncodeQos, rsslDecodeQos
 */
typedef struct {
	RsslUInt8	 timeliness;		/*!< @brief Timeliness information, indicating data age (e.g. real-time, delayed). Should be populated from \ref RsslQosTimeliness enum. */
	RsslUInt8	 rate;				/*!< @brief Rate information, indicating data period of change (e.g. tick-by-tick, conflated).  Should be populated from \ref RsslQosRates enum. */
	RsslUInt8    dynamic:1;			/*!< @brief If RSSL_TRUE, Rssl Qos is dynamic (used to describe the changeability of the quality of service, typically over the life of a data stream) */

	RsslUInt16   timeInfo;			/*!< @brief Indicates specific timeliness information, describing the age of data.  This information is only applicable when RsslQos::timeliness is set to ::RSSL_QOS_TIME_DELAYED. */
	RsslUInt16   rateInfo;			/*!< @brief Indicates specific rate information, describing the period of change associated with the data.  This information is only applicable when RsslQos::rate is set to ::RSSL_QOS_RATE_TIME_CONFLATED and is specified in milliseconds. */

} RsslQos;

/**
 * @brief RsslQos static initializer
 * @see RsslQos, rsslClearQos
 */
#define RSSL_INIT_QOS { 0, 0, 0, 0, 0 }

/**
 * @brief Clears RsslQos structure
 * @see RsslQos, RSSL_INIT_QOS
 */
#define rsslClearQos(pQos) ( void )( (pQos)->timeliness = 0, \
							(pQos)->rate = 0, \
							(pQos)->dynamic = 0, \
							(pQos)->timeInfo = 0, \
							(pQos)->rateInfo = 0  )


/** 
 * @brief RsslQos Timeliness enumerations, used to convey information about the age of the data.
 */
typedef enum {
	RSSL_QOS_TIME_UNSPECIFIED	  = 0,	/*!< Timeliness is unspecified, indicates initialized structure and not intended to be encoded. */
	RSSL_QOS_TIME_REALTIME	  	  = 1,	/*!< Timeliness is Realtime, indicates information is updated as soon as new information becomes available */
	RSSL_QOS_TIME_DELAYED_UNKNOWN = 2,	/*!< Timeliness is delayed, but delay time info is unknown */
	RSSL_QOS_TIME_DELAYED		  = 3	/*!< Timeliness is delayed by a specific amount of time, where amount is provided in RsslQos::timeInfo */
} RsslQosTimeliness;

/** 
 * @brief General OMM strings associated with the different Qos timeliness.
 * @see RsslQosTimeliness, rsslQosTimelinessToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_QOS_TIME_UNSPECIFIED = { 11, (char*)"Unspecified" };
static const RsslBuffer RSSL_OMMSTR_QOS_TIME_REALTIME = { 8, (char*)"Realtime" };
static const RsslBuffer RSSL_OMMSTR_QOS_TIME_DELAYED_UNKNOWN = { 14, (char*)"DelayedUnknown" };
static const RsslBuffer RSSL_OMMSTR_QOS_TIME_DELAYED = { 7, (char*)"Delayed" };

/** 
 * @brief RsslQos Rate enumerations, used to convey information about the data’s period of change.
 */
typedef enum {
	RSSL_QOS_RATE_UNSPECIFIED		= 0,	/*!< Rate is Unspecified, indicates initialized structure and not intended to be encoded */
	RSSL_QOS_RATE_TICK_BY_TICK		= 1,	/*!< Rate is Tick By Tick, indicates every change to information is conveyed */
	RSSL_QOS_RATE_JIT_CONFLATED		= 2,	/*!< Rate is Just In Time Conflated, indicates extreme bursts of data may be conflated */
	RSSL_QOS_RATE_TIME_CONFLATED	= 3		/*!< Rate is conflated by a specific amount of time (in ms), where conflation time is provided in RsslQos::rateInfo */
} RsslQosRates;

/** 
 * @brief General OMM strings associated with the different Qos rates.
 * @see RsslQosRates, rsslQosRateToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_QOS_RATE_UNSPECIFIED = { 11, (char*)"Unspecified" };
static const RsslBuffer RSSL_OMMSTR_QOS_RATE_TICK_BY_TICK = { 10, (char*)"TickByTick" };
static const RsslBuffer RSSL_OMMSTR_QOS_RATE_JIT_CONFLATED = { 12, (char*)"JitConflated" };
static const RsslBuffer RSSL_OMMSTR_QOS_RATE_TIME_CONFLATED = { 13, (char*)"TimeConflated" };

/** 
 * @}
 */

/**
 *	@addtogroup RsslQosUtils
 *	@{
 */

/**
 * @brief Provide string representation for an RsslQos::timeliness
 * @param value \ref RsslQosTimeliness enumeration to convert to string
 * @return const char* representation of corresponding \ref RsslQosTimeliness
 * @see RsslQosTimeliness, RsslQos
 */
RTR_C_ALWAYS_INLINE const char* rsslQosTimelinessToString(RsslUInt8 value)
{
	switch (value)
	{
	case RSSL_QOS_TIME_UNSPECIFIED:		return "Unspecified";
	case RSSL_QOS_TIME_REALTIME:		return "Realtime";
	case RSSL_QOS_TIME_DELAYED_UNKNOWN:	return "DelayedByUnknown";
	case RSSL_QOS_TIME_DELAYED:			return "DelayedByTimeInfo";
	default:							return "Unknown QosTimeliness";
	}
}

/**
 * @brief Provide general OMM string representation for an RsslQos::timeliness
 * @param value \ref RsslQosTimeliness enumeration to convert to string
 * @return const char* representation of corresponding \ref RsslQosTimeliness
 * @see RsslQosTimeliness, RsslQos
 */
RTR_C_ALWAYS_INLINE const char* rsslQosTimelinessToOmmString(RsslUInt8 value)
{
	switch (value)
	{
	case RSSL_QOS_TIME_UNSPECIFIED:		return RSSL_OMMSTR_QOS_TIME_UNSPECIFIED.data;
	case RSSL_QOS_TIME_REALTIME:		return RSSL_OMMSTR_QOS_TIME_REALTIME.data;
	case RSSL_QOS_TIME_DELAYED_UNKNOWN:	return RSSL_OMMSTR_QOS_TIME_DELAYED_UNKNOWN.data;
	case RSSL_QOS_TIME_DELAYED:			return RSSL_OMMSTR_QOS_TIME_DELAYED.data;
	default:							return NULL;
	}
}

/**
 * @brief Provide string representation for an RsslQos::rate
 * @param value \ref RsslQosRates enumeration to convert to string
 * @return const char* representation of corresponding \ref RsslQosRates
 * @see RsslQosRates, RsslQos
 */
RTR_C_ALWAYS_INLINE const char* rsslQosRateToString(RsslUInt8 value)
{
	switch (value)
	{
	case RSSL_QOS_RATE_UNSPECIFIED:		return "Unspecified";
	case RSSL_QOS_RATE_TICK_BY_TICK:	return "TickByTick";
	case RSSL_QOS_RATE_JIT_CONFLATED:	return "JustInTimeConflated";
	case RSSL_QOS_RATE_TIME_CONFLATED:	return "ConflatedByRateInfo";
	default:							return "Unknown QosRate";
	}
}

/**
 * @brief Provide general OMM string representation for an RsslQos::rate
 * @param value \ref RsslQosRates enumeration to convert to string
 * @return const char* representation of corresponding \ref RsslQosRates
 * @see RsslQosRates, RsslQos
 */
RTR_C_ALWAYS_INLINE const char* rsslQosRateToOmmString(RsslUInt8 value)
{
	switch (value)
	{
	case RSSL_QOS_RATE_UNSPECIFIED:		return RSSL_OMMSTR_QOS_RATE_UNSPECIFIED.data;
	case RSSL_QOS_RATE_TICK_BY_TICK:	return RSSL_OMMSTR_QOS_RATE_TICK_BY_TICK.data;
	case RSSL_QOS_RATE_JIT_CONFLATED:	return RSSL_OMMSTR_QOS_RATE_JIT_CONFLATED.data;
	case RSSL_QOS_RATE_TIME_CONFLATED:	return RSSL_OMMSTR_QOS_RATE_TIME_CONFLATED.data;
	default:							return NULL;
	}
}

/**
 * @brief Provide string representation for an RsslQos, including all RsslQos members
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point to memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param pQos Fully populated RsslQos structure
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_FAILURE otherwise
 * @see RsslQos
 */
RTR_C_ALWAYS_INLINE RsslRet rsslQosToString(RsslBuffer *oBuffer, RsslQos *pQos)
{
	int length = 0;

	length = snprintf(oBuffer->data, oBuffer->length, "Qos: %s/%s/%s - timeInfo: %d - rateInfo: %d",
		rsslQosTimelinessToString(pQos->timeliness),
		rsslQosRateToString(pQos->rate),
		pQos->dynamic ? "Dynamic" : "Static",
		pQos->timeInfo,
		pQos->rateInfo);
	if (length >= 0 && length < (int)oBuffer->length)
	{
		oBuffer->length = length;
		return RSSL_RET_SUCCESS;
	}
	else
	{
		return RSSL_RET_FAILURE;
	}
}


/**
 * @brief Checks equality of two RsslQos structures
 * @param qos1 Pointer to the left hand side RsslQos structure.
 * @param qos2 Pointer to the right hand side RsslQos structure.
 * @return RsslBool RSSL_TRUE if both RsslQos structures are equal, RSSL_FALSE otherwise
 * @see RsslQos
 */
RTR_C_ALWAYS_INLINE RsslBool rsslQosIsEqual (const RsslQos *qos1, const RsslQos *qos2)
{
	return ((qos1->rate == qos2->rate) && (qos1->timeliness == qos2->timeliness) &&
			(qos1->rateInfo == qos2->rateInfo) && (qos1->timeInfo == qos2->timeInfo));
}


/**
 * @brief Checks if the new QoS is better than the old QoS, where real-time is more desirable than delayed, tick-by-tick is more desirable than conflated.
 * @param newQos Pointer to the RsslQos representing the new quality of service.
 * @param oldQos Pointer to the RsslQos representing the old quality of service.
 * @return RsslBool RSSL_TRUE if newQos is better, RSSL_FALSE otherwise
 * @see RsslQos
 */
RSSL_API RsslBool rsslQosIsBetter(const RsslQos *newQos, const RsslQos *oldQos);




/**
 * @brief Checks if the QoS is in the range defined using the best and worst QoS.
 * @param bestQos Pointer to the RsslQos representing the best QoS within the desired range
 * @param worstQos Pointer to the RsslQos representing the worst QoS within the desired range
 * @param Qos Pointer to the RsslQos to check against the specified QoS range
 * @return RsslBool RSSL_TRUE if Qos is within the range, RSSL_FALSE otherwise
 * @see RsslQos
 */
RSSL_API RsslBool rsslQosIsInRange(const RsslQos *bestQos, const RsslQos *worstQos, const RsslQos *Qos);



/**
 * @brief Copies source QoS into target QoS 
 * @param targetQos Pointer to the RsslQos to copy into
 * @param sourceQos Pointer to the RsslQos to copy from
 * @see RsslQos
 */
RTR_C_ALWAYS_INLINE void rsslCopyQos(RsslQos *targetQos, const RsslQos *sourceQos)
{
	targetQos->dynamic = sourceQos->dynamic;
	targetQos->rate = sourceQos->rate;
	targetQos->rateInfo = sourceQos->rateInfo;
	targetQos->timeInfo = sourceQos->timeInfo;
	targetQos->timeliness = sourceQos->timeliness;
}

/**
 * @}
 */


#ifdef __cplusplus
}
#endif 

#endif
